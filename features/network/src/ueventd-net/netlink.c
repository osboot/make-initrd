// SPDX-License-Identifier: GPL-3.0-or-later

#include "config.h"

#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_addr.h>
#include <linux/if_link.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "rd/logging.h"
#include "ueventd-net.h"

#define NL_RECV_BUFSIZE  (64 * 1024)
#define NL_SOCK_RCVBUF   (512 * 1024)

static void send_dump_request(struct netlink_ctx *ctx, uint16_t type)
{
	struct {
		struct nlmsghdr hdr;
		struct rtgenmsg msg;
	} req;
	struct sockaddr_nl addr;

	memset(&req, 0, sizeof(req));
	req.hdr.nlmsg_len = sizeof(req);
	req.hdr.nlmsg_type = type;
	req.hdr.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
	req.hdr.nlmsg_seq = ++ctx->seq;
	req.msg.rtgen_family = AF_UNSPEC;

	ctx->dump_seq = req.hdr.nlmsg_seq;

	memset(&addr, 0, sizeof(addr));
	addr.nl_family = AF_NETLINK;

	if (sendto(ctx->fd, &req, sizeof(req), 0,
	           (struct sockaddr *) &addr, sizeof(addr)) < 0)
		rd_fatal("sendto(netlink dump type=%u): %m", (unsigned int) type);
}

static void advance_dump(struct netlink_ctx *ctx)
{
	switch (ctx->dump_state) {
		case DUMP_LINK:
			ctx->dump_state = DUMP_ADDR;
			send_dump_request(ctx, RTM_GETADDR);
			break;
		case DUMP_ADDR:
			ctx->dump_state = DUMP_ROUTE;
			send_dump_request(ctx, RTM_GETROUTE);
			break;
		case DUMP_ROUTE:
			ctx->dump_state = DUMP_IDLE;
			net_state_sweep_unseen();
			net_state_sync_all();
			rd_dbg("netlink dump/sync complete");
			break;
		case DUMP_IDLE:
			break;
	}
}

void netlink_start_dump(struct netlink_ctx *ctx)
{
	net_state_mark_unseen();
	ctx->dump_state = DUMP_LINK;
	send_dump_request(ctx, RTM_GETLINK);
}

int netlink_open(struct netlink_ctx *ctx)
{
	struct sockaddr_nl local;
	int rcvbuf = NL_SOCK_RCVBUF;
	static const int groups[] = {
		RTNLGRP_LINK,
		RTNLGRP_IPV4_IFADDR,
		RTNLGRP_IPV6_IFADDR,
		RTNLGRP_IPV4_ROUTE,
		RTNLGRP_IPV6_ROUTE,
	};

	memset(ctx, 0, sizeof(*ctx));
	ctx->fd = socket(AF_NETLINK, SOCK_RAW | SOCK_CLOEXEC | SOCK_NONBLOCK, NETLINK_ROUTE);
	if (ctx->fd < 0)
		rd_fatal("socket(AF_NETLINK, NETLINK_ROUTE): %m");

	if (setsockopt(ctx->fd, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(rcvbuf)) < 0)
		rd_err("setsockopt(SO_RCVBUF): %m");

	memset(&local, 0, sizeof(local));
	local.nl_family = AF_NETLINK;

	if (bind(ctx->fd, (struct sockaddr *) &local, sizeof(local)) < 0)
		rd_fatal("bind(AF_NETLINK): %m");

	for (size_t i = 0; i < sizeof(groups) / sizeof(groups[0]); i++) {
		int grp = groups[i];
		if (setsockopt(ctx->fd, SOL_NETLINK, NETLINK_ADD_MEMBERSHIP,
		               &grp, sizeof(grp)) < 0)
			rd_err("setsockopt(NETLINK_ADD_MEMBERSHIP, %d): %m", grp);
	}

	return ctx->fd;
}

void netlink_close(struct netlink_ctx *ctx)
{
	if (ctx->fd >= 0) {
		close(ctx->fd);
		ctx->fd = -1;
	}
}

static bool rta_ok_safe(const struct rtattr *attr, size_t remaining)
{
	return attr != NULL &&
	       remaining >= sizeof(*attr) &&
	       attr->rta_len >= sizeof(*attr) &&
	       attr->rta_len <= remaining;
}

static struct rtattr *rta_next_safe(struct rtattr *attr, size_t *remaining)
{
	size_t aligned = RTA_ALIGN(attr->rta_len);

	if (aligned >= *remaining) {
		*remaining = 0;
		return NULL;
	}
	*remaining -= aligned;
	return (struct rtattr *) (void *) ((char *) attr + aligned);
}

static void handle_link(struct nlmsghdr *hdr, bool defer_emit)
{
	struct ifinfomsg msg;
	struct rtattr *attr;
	size_t attrlen;
	char ifname[IFNAMSIZ] = "";

	if (hdr->nlmsg_len < NLMSG_LENGTH(sizeof(msg)))
		return;

	memcpy(&msg, NLMSG_DATA(hdr), sizeof(msg));

	if (hdr->nlmsg_type == RTM_DELLINK) {
		net_state_link_remove(msg.ifi_index, defer_emit);
		return;
	}

	attrlen = IFLA_PAYLOAD(hdr);
	for (attr = IFLA_RTA((struct ifinfomsg *) NLMSG_DATA(hdr));
	     rta_ok_safe(attr, attrlen);
	     attr = rta_next_safe(attr, &attrlen)) {
		if (attr->rta_type == IFLA_IFNAME) {
			size_t len = RTA_PAYLOAD(attr);
			if (len >= sizeof(ifname))
				len = sizeof(ifname) - 1;
			memcpy(ifname, RTA_DATA(attr), len);
			ifname[len] = '\0';
		}
	}

	net_state_link_update(msg.ifi_index, ifname, msg.ifi_flags, defer_emit);
}

static void handle_addr(struct nlmsghdr *hdr, bool defer_emit)
{
	struct ifaddrmsg msg;
	struct rtattr *attr;
	size_t attrlen;
	uint8_t addr[16] = { 0 };
	bool has_local = false;
	bool has_addr = false;
	uint32_t ifa_flags;
	size_t expected_len;

	if (hdr->nlmsg_len < NLMSG_LENGTH(sizeof(msg)))
		return;

	memcpy(&msg, NLMSG_DATA(hdr), sizeof(msg));

	if (msg.ifa_family != AF_INET && msg.ifa_family != AF_INET6)
		return;

	expected_len = (msg.ifa_family == AF_INET6) ? 16 : 4;
	ifa_flags = msg.ifa_flags;

	attrlen = IFA_PAYLOAD(hdr);
	for (attr = IFA_RTA((struct ifaddrmsg *) NLMSG_DATA(hdr));
	     rta_ok_safe(attr, attrlen);
	     attr = rta_next_safe(attr, &attrlen)) {
		switch (attr->rta_type) {
			case IFA_LOCAL:
				if (RTA_PAYLOAD(attr) == expected_len) {
					memcpy(addr, RTA_DATA(attr), expected_len);
					has_local = true;
				}
				break;
			case IFA_ADDRESS:
				if (!has_local && RTA_PAYLOAD(attr) == expected_len) {
					memcpy(addr, RTA_DATA(attr), expected_len);
					has_addr = true;
				}
				break;
#ifdef IFA_FLAGS
			case IFA_FLAGS:
				if (RTA_PAYLOAD(attr) == sizeof(uint32_t))
					memcpy(&ifa_flags, RTA_DATA(attr), sizeof(uint32_t));
				break;
#endif
		}
	}

	if (!has_local && !has_addr)
		return;

	if (hdr->nlmsg_type == RTM_DELADDR) {
		net_state_addr_remove((int) msg.ifa_index, msg.ifa_family,
		                      msg.ifa_prefixlen, addr, defer_emit);
		return;
	}

	net_state_addr_update((int) msg.ifa_index, msg.ifa_family,
	                      msg.ifa_prefixlen, msg.ifa_scope,
	                      ifa_flags, addr, defer_emit);
}

static bool ipv6_route_is_ignored(uint8_t dst_len, const uint8_t *dst)
{
	/* Ignore link-local fe80::/10 and multicast ff00::/8 routes */
	if (dst_len >= 10 && dst[0] == 0xfe && (dst[1] & 0xc0) == 0x80)
		return true;
	if (dst_len >= 8 && dst[0] == 0xff)
		return true;
	return false;
}

static void handle_route(struct nlmsghdr *hdr, bool defer_emit)
{
	struct rtmsg msg;
	struct rtattr *attr;
	size_t attrlen;
	uint32_t table;
	int oif = 0;
	uint8_t dst[16] = { 0 };
	uint8_t gw[16] = { 0 };
	bool has_gw = false;
	uint32_t priority = 0;
	size_t expected_len;

	if (hdr->nlmsg_len < NLMSG_LENGTH(sizeof(msg)))
		return;

	memcpy(&msg, NLMSG_DATA(hdr), sizeof(msg));

	if (msg.rtm_family != AF_INET && msg.rtm_family != AF_INET6)
		return;

	if (msg.rtm_type != RTN_UNICAST)
		return;

	table = msg.rtm_table;
	expected_len = (msg.rtm_family == AF_INET6) ? 16 : 4;

	attrlen = RTM_PAYLOAD(hdr);
	for (attr = RTM_RTA((struct rtmsg *) NLMSG_DATA(hdr));
	     rta_ok_safe(attr, attrlen);
	     attr = rta_next_safe(attr, &attrlen)) {
		switch (attr->rta_type) {
			case RTA_TABLE:
				if (RTA_PAYLOAD(attr) == sizeof(uint32_t))
					memcpy(&table, RTA_DATA(attr), sizeof(uint32_t));
				break;
			case RTA_OIF:
				if (RTA_PAYLOAD(attr) == sizeof(int))
					memcpy(&oif, RTA_DATA(attr), sizeof(int));
				break;
			case RTA_DST:
				if (RTA_PAYLOAD(attr) == expected_len)
					memcpy(dst, RTA_DATA(attr), expected_len);
				break;
			case RTA_GATEWAY:
				if (RTA_PAYLOAD(attr) == expected_len) {
					memcpy(gw, RTA_DATA(attr), expected_len);
					has_gw = true;
				}
				break;
			case RTA_PRIORITY:
				if (RTA_PAYLOAD(attr) == sizeof(uint32_t))
					memcpy(&priority, RTA_DATA(attr), sizeof(uint32_t));
				break;
		}
	}

	if (table != RT_TABLE_MAIN || oif <= 0)
		return;

	if (msg.rtm_family == AF_INET6 && ipv6_route_is_ignored(msg.rtm_dst_len, dst))
		return;

	if (hdr->nlmsg_type == RTM_DELROUTE) {
		net_state_route_remove(oif, msg.rtm_family, msg.rtm_dst_len,
		                       msg.rtm_protocol, msg.rtm_scope,
		                       dst, gw, has_gw, priority, defer_emit);
		return;
	}

	net_state_route_update(oif, msg.rtm_family, msg.rtm_dst_len,
	                       msg.rtm_protocol, msg.rtm_scope,
	                       dst, gw, has_gw, priority, defer_emit);
}

void netlink_dispatch(struct netlink_ctx *ctx)
{
	union {
		char raw[NL_RECV_BUFSIZE];
		struct nlmsghdr nh;
	} buf;

	while (1) {
		ssize_t n = recv(ctx->fd, buf.raw, sizeof(buf.raw), 0);
		if (n < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				return;
			if (errno == EINTR)
				continue;
			if (errno == ENOBUFS) {
				rd_err("netlink socket overflow (ENOBUFS), resyncing state");
				netlink_start_dump(ctx);
				return;
			}
			rd_fatal("recv(netlink): %m");
		}
		if (n == 0)
			return;

		size_t remaining = (size_t) n;
		struct nlmsghdr *hdr = &buf.nh;

		for (; NLMSG_OK(hdr, remaining); hdr = NLMSG_NEXT(hdr, remaining)) {
			bool defer_emit = (ctx->dump_state != DUMP_IDLE);

			switch (hdr->nlmsg_type) {
				case RTM_NEWLINK:
				case RTM_DELLINK:
					handle_link(hdr, defer_emit);
					break;
				case RTM_NEWADDR:
				case RTM_DELADDR:
					handle_addr(hdr, defer_emit);
					break;
				case RTM_NEWROUTE:
				case RTM_DELROUTE:
					handle_route(hdr, defer_emit);
					break;
				case NLMSG_DONE:
					if (ctx->dump_state != DUMP_IDLE &&
					    hdr->nlmsg_seq == ctx->dump_seq)
						advance_dump(ctx);
					break;
				case NLMSG_ERROR: {
					struct nlmsgerr nlerr;
					if (hdr->nlmsg_len >= NLMSG_LENGTH(sizeof(nlerr))) {
						memcpy(&nlerr, NLMSG_DATA(hdr), sizeof(nlerr));
						if (nlerr.error != 0)
							rd_err("netlink error: %s", strerror(-nlerr.error));
					}
					if (ctx->dump_state != DUMP_IDLE &&
					    hdr->nlmsg_seq == ctx->dump_seq)
						advance_dump(ctx);
					break;
				}
				case NLMSG_NOOP:
					break;
				default:
					rd_dbg("ignoring nlmsg_type=%u", (unsigned int) hdr->nlmsg_type);
					break;
			}
		}
	}
}
