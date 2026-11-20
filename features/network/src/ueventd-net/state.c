// SPDX-License-Identifier: GPL-3.0-or-later

#include "config.h"

#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_addr.h>
#include <linux/rtnetlink.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "rd/logging.h"
#include "rd/memory.h"
#include "ueventd-net.h"

static LIST_HEAD(ifaces);

static size_t family_addr_len(uint8_t family)
{
	return (family == AF_INET6) ? 16 : 4;
}

static bool iface_has_carrier(unsigned int flags)
{
	if (flags & IFF_LOOPBACK)
		return (flags & IFF_UP) != 0;
	return (flags & IFF_LOWER_UP) != 0;
}

static bool addr_is_usable(uint8_t family, uint8_t scope, uint32_t flags)
{
	if (scope == RT_SCOPE_NOWHERE)
		return false;

	if (flags & (IFA_F_TENTATIVE | IFA_F_DADFAILED))
		return false;

	if (family == AF_INET6 && scope == RT_SCOPE_LINK)
		return false;

	return (family == AF_INET || family == AF_INET6);
}

static struct net_iface *iface_find(int ifindex)
{
	struct net_iface *iface;

	list_for_each_entry(iface, &ifaces, list) {
		if (iface->ifindex == ifindex)
			return iface;
	}
	return NULL;
}

static struct net_iface *iface_create(int ifindex, const char *ifname)
{
	struct net_iface *iface = rd_calloc_or_die(1, sizeof(*iface));

	iface->ifindex = ifindex;
	snprintf(iface->ifname, sizeof(iface->ifname), "%s", ifname);
	INIT_LIST_HEAD(&iface->addrs);
	INIT_LIST_HEAD(&iface->routes);
	list_add_tail(&iface->list, &ifaces);

	return iface;
}

static void iface_free(struct net_iface *iface)
{
	struct net_addr *a, *an;
	struct net_route *r, *rn;

	list_for_each_entry_safe(a, an, &iface->addrs, list) {
		list_del(&a->list);
		free(a);
	}
	list_for_each_entry_safe(r, rn, &iface->routes, list) {
		list_del(&r->list);
		free(r);
	}

	list_del(&iface->list);
	free(iface);
}

static void emit_carrier(struct net_iface *iface, bool up)
{
	const char *ev = up ? "carrier.up" : "carrier.down";

	rd_info("%s (ifindex=%d): %s", iface->ifname, iface->ifindex, ev);
	rd_uevent_emit(&event_queue, ev,
	               "NET_IF='%s'\n"
	               "NET_IFINDEX='%d'\n",
	               iface->ifname, iface->ifindex);
}

static void emit_addr(struct net_iface *iface, int ip_ver, bool ready)
{
	const char *ev = ready ? "addr.ready" : "addr.lost";

	rd_info("%s (ifindex=%d): %s (ipv%d)", iface->ifname, iface->ifindex, ev, ip_ver);
	rd_uevent_emit(&event_queue, ev,
	               "NET_IF='%s'\n"
	               "NET_IFINDEX='%d'\n"
	               "IP_VER='%d'\n",
	               iface->ifname, iface->ifindex, ip_ver);
}

static void emit_route(struct net_iface *iface, int ip_ver, bool ready, bool has_default)
{
	const char *ev = ready ? "route.ready" : "route.lost";

	rd_info("%s (ifindex=%d): %s (ipv%d, default=%d)",
	        iface->ifname, iface->ifindex, ev, ip_ver, has_default ? 1 : 0);
	rd_uevent_emit(&event_queue, ev,
	               "NET_IF='%s'\n"
	               "NET_IFINDEX='%d'\n"
	               "IP_VER='%d'\n"
	               "HAS_DEFAULT='%d'\n",
	               iface->ifname, iface->ifindex, ip_ver, has_default ? 1 : 0);
}

static void iface_sync(struct net_iface *iface)
{
	struct net_addr *a;
	struct net_route *r;
	bool has_addr4 = false;
	bool has_addr6 = false;
	bool has_route4 = false;
	bool has_route6 = false;
	bool has_default4 = false;
	bool has_default6 = false;

	if (!iface->ifname[0])
		return;

	list_for_each_entry(a, &iface->addrs, list) {
		if (a->family == AF_INET)
			has_addr4 = true;
		else if (a->family == AF_INET6)
			has_addr6 = true;
	}

	list_for_each_entry(r, &iface->routes, list) {
		if (r->family == AF_INET) {
			has_route4 = true;
			if (r->dst_len == 0)
				has_default4 = true;
		} else if (r->family == AF_INET6) {
			has_route6 = true;
			if (r->dst_len == 0)
				has_default6 = true;
		}
	}

	if (iface->carrier != iface->emitted_carrier) {
		iface->emitted_carrier = iface->carrier;
		emit_carrier(iface, iface->carrier);
	}

	if (has_addr4 != iface->emitted_addr4) {
		iface->emitted_addr4 = has_addr4;
		emit_addr(iface, 4, has_addr4);
	}

	if (has_addr6 != iface->emitted_addr6) {
		iface->emitted_addr6 = has_addr6;
		emit_addr(iface, 6, has_addr6);
	}

	if (has_route4 != iface->emitted_route4 ||
	    (has_route4 && has_default4 != iface->emitted_default4)) {
		iface->emitted_route4 = has_route4;
		iface->emitted_default4 = has_route4 ? has_default4 : false;
		emit_route(iface, 4, has_route4, iface->emitted_default4);
	}

	if (has_route6 != iface->emitted_route6 ||
	    (has_route6 && has_default6 != iface->emitted_default6)) {
		iface->emitted_route6 = has_route6;
		iface->emitted_default6 = has_route6 ? has_default6 : false;
		emit_route(iface, 6, has_route6, iface->emitted_default6);
	}
}

void net_state_init(void)
{
	INIT_LIST_HEAD(&ifaces);
}

void net_state_cleanup(void)
{
	struct net_iface *iface, *n;

	list_for_each_entry_safe(iface, n, &ifaces, list) {
		iface_free(iface);
	}
}

void net_state_mark_unseen(void)
{
	struct net_iface *iface;
	struct net_addr *a;
	struct net_route *r;

	list_for_each_entry(iface, &ifaces, list) {
		iface->seen = false;
		list_for_each_entry(a, &iface->addrs, list)
			a->seen = false;
		list_for_each_entry(r, &iface->routes, list)
			r->seen = false;
	}
}

void net_state_sweep_unseen(void)
{
	struct net_iface *iface, *in;
	struct net_addr *a, *an;
	struct net_route *r, *rn;

	list_for_each_entry_safe(iface, in, &ifaces, list) {
		if (!iface->seen) {
			iface->carrier = false;
			list_for_each_entry_safe(a, an, &iface->addrs, list) {
				list_del(&a->list);
				free(a);
			}
			list_for_each_entry_safe(r, rn, &iface->routes, list) {
				list_del(&r->list);
				free(r);
			}
			iface_sync(iface);
			iface_free(iface);
			continue;
		}

		list_for_each_entry_safe(a, an, &iface->addrs, list) {
			if (!a->seen) {
				list_del(&a->list);
				free(a);
			}
		}

		list_for_each_entry_safe(r, rn, &iface->routes, list) {
			if (!r->seen) {
				list_del(&r->list);
				free(r);
			}
		}
	}
}

void net_state_sync_all(void)
{
	struct net_iface *iface;

	list_for_each_entry(iface, &ifaces, list)
		iface_sync(iface);
}

void net_state_link_update(int ifindex, const char *ifname,
                           unsigned int flags, bool defer_emit)
{
	struct net_iface *iface = iface_find(ifindex);
	bool renamed = false;

	if (!iface) {
		iface = iface_create(ifindex, ifname);
	} else if (ifname[0] && strcmp(iface->ifname, ifname) != 0) {
		rd_info("rename (ifindex=%d): %s -> %s", ifindex, iface->ifname, ifname);
		snprintf(iface->ifname, sizeof(iface->ifname), "%s", ifname);
		renamed = true;
	}

	iface->seen = true;
	iface->flags = flags;
	iface->carrier = iface_has_carrier(flags);

	if (renamed) {
		iface->emitted_carrier = false;
		iface->emitted_addr4 = false;
		iface->emitted_addr6 = false;
		iface->emitted_route4 = false;
		iface->emitted_route6 = false;
		iface->emitted_default4 = false;
		iface->emitted_default6 = false;
	}

	if (!defer_emit)
		iface_sync(iface);
}

void net_state_link_remove(int ifindex, bool defer_emit)
{
	struct net_iface *iface = iface_find(ifindex);
	struct net_addr *a, *an;
	struct net_route *r, *rn;

	if (!iface)
		return;

	iface->carrier = false;
	list_for_each_entry_safe(a, an, &iface->addrs, list) {
		list_del(&a->list);
		free(a);
	}
	list_for_each_entry_safe(r, rn, &iface->routes, list) {
		list_del(&r->list);
		free(r);
	}

	if (!defer_emit)
		iface_sync(iface);

	iface_free(iface);
}

static struct net_addr *addr_find(struct net_iface *iface, uint8_t family,
                                  uint8_t prefixlen, const uint8_t *addr)
{
	struct net_addr *a;
	size_t len = family_addr_len(family);

	list_for_each_entry(a, &iface->addrs, list) {
		if (a->family == family &&
		    a->prefixlen == prefixlen &&
		    memcmp(a->addr, addr, len) == 0)
			return a;
	}
	return NULL;
}

void net_state_addr_update(int ifindex, uint8_t family, uint8_t prefixlen,
                           uint8_t scope, uint32_t flags,
                           const uint8_t *addr, bool defer_emit)
{
	struct net_iface *iface = iface_find(ifindex);
	struct net_addr *a;

	if (!iface)
		return;

	a = addr_find(iface, family, prefixlen, addr);

	if (!addr_is_usable(family, scope, flags)) {
		if (a) {
			list_del(&a->list);
			free(a);
			if (!defer_emit)
				iface_sync(iface);
		}
		return;
	}

	if (!a) {
		a = rd_calloc_or_die(1, sizeof(*a));
		a->family = family;
		a->prefixlen = prefixlen;
		a->scope = scope;
		memcpy(a->addr, addr, family_addr_len(family));
		list_add_tail(&a->list, &iface->addrs);
	} else {
		a->scope = scope;
	}

	a->seen = true;

	if (!defer_emit)
		iface_sync(iface);
}

void net_state_addr_remove(int ifindex, uint8_t family, uint8_t prefixlen,
                           const uint8_t *addr, bool defer_emit)
{
	struct net_iface *iface = iface_find(ifindex);
	struct net_addr *a;

	if (!iface)
		return;

	a = addr_find(iface, family, prefixlen, addr);
	if (!a)
		return;

	list_del(&a->list);
	free(a);

	if (!defer_emit)
		iface_sync(iface);
}

static struct net_route *route_find(struct net_iface *iface, uint8_t family,
                                    uint8_t dst_len, uint8_t protocol,
                                    uint8_t scope, const uint8_t *dst,
                                    const uint8_t *gw, bool has_gw,
                                    uint32_t priority)
{
	struct net_route *r;
	size_t len = family_addr_len(family);

	list_for_each_entry(r, &iface->routes, list) {
		if (r->family != family ||
		    r->dst_len != dst_len ||
		    r->protocol != protocol ||
		    r->scope != scope ||
		    r->has_gw != has_gw ||
		    r->priority != priority)
			continue;

		if (memcmp(r->dst, dst, len) != 0)
			continue;

		if (has_gw && memcmp(r->gw, gw, len) != 0)
			continue;

		return r;
	}
	return NULL;
}

void net_state_route_update(int ifindex, uint8_t family, uint8_t dst_len,
                            uint8_t protocol, uint8_t scope,
                            const uint8_t *dst, const uint8_t *gw,
                            bool has_gw, uint32_t priority,
                            bool defer_emit)
{
	struct net_iface *iface = iface_find(ifindex);
	struct net_route *r;
	size_t len = family_addr_len(family);

	if (!iface)
		return;

	r = route_find(iface, family, dst_len, protocol, scope, dst, gw, has_gw, priority);
	if (!r) {
		r = rd_calloc_or_die(1, sizeof(*r));
		r->family = family;
		r->dst_len = dst_len;
		r->protocol = protocol;
		r->scope = scope;
		memcpy(r->dst, dst, len);
		if (has_gw && gw) {
			memcpy(r->gw, gw, len);
			r->has_gw = true;
		}
		r->priority = priority;
		list_add_tail(&r->list, &iface->routes);
	}

	r->seen = true;

	if (!defer_emit)
		iface_sync(iface);
}

void net_state_route_remove(int ifindex, uint8_t family, uint8_t dst_len,
                            uint8_t protocol, uint8_t scope,
                            const uint8_t *dst, const uint8_t *gw,
                            bool has_gw, uint32_t priority,
                            bool defer_emit)
{
	struct net_iface *iface = iface_find(ifindex);
	struct net_route *r;

	if (!iface)
		return;

	r = route_find(iface, family, dst_len, protocol, scope, dst, gw, has_gw, priority);
	if (!r)
		return;

	list_del(&r->list);
	free(r);

	if (!defer_emit)
		iface_sync(iface);
}
