// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef _UEVENTD_NET_H
#define _UEVENTD_NET_H 1

#include <linux/if.h>
#include <stdint.h>
#include <stdbool.h>

#include "compiler.h"
#include "rd/list.h"
#include "rd/uevent.h"

struct net_addr {
	struct list_head list;
	uint8_t family;
	uint8_t prefixlen;
	uint8_t scope;
	uint8_t addr[16];
	bool seen;
};

struct net_route {
	struct list_head list;
	uint8_t family;
	uint8_t dst_len;
	uint8_t protocol;
	uint8_t scope;
	uint8_t dst[16];
	uint8_t gw[16];
	bool has_gw;
	uint32_t priority;
	bool seen;
};

struct net_iface {
	struct list_head list;
	int ifindex;
	char ifname[IFNAMSIZ];
	unsigned int flags;
	bool carrier;
	bool seen;

	struct list_head addrs;
	struct list_head routes;

	bool emitted_carrier;
	bool emitted_addr4;
	bool emitted_addr6;
	bool emitted_route4;
	bool emitted_route6;
	bool emitted_default4;
	bool emitted_default6;
};

enum nl_dump_state {
	DUMP_IDLE = 0,
	DUMP_LINK,
	DUMP_ADDR,
	DUMP_ROUTE,
};

struct netlink_ctx {
	int fd;
	uint32_t seq;
	enum nl_dump_state dump_state;
	uint32_t dump_seq;
};

extern struct rd_uevent_queue event_queue;

/* state.c */
void net_state_init(void);
void net_state_cleanup(void);
void net_state_mark_unseen(void);
void net_state_sweep_unseen(void);
void net_state_sync_all(void);

void net_state_link_update(int ifindex, const char *ifname,
                           unsigned int flags, bool defer_emit) _rdonly_ptr_(2);
void net_state_link_remove(int ifindex, bool defer_emit);

void net_state_addr_update(int ifindex, uint8_t family, uint8_t prefixlen,
                           uint8_t scope, uint32_t flags,
                           const uint8_t *addr, bool defer_emit) _rdonly_ptr_(6);
void net_state_addr_remove(int ifindex, uint8_t family, uint8_t prefixlen,
                           const uint8_t *addr, bool defer_emit) _rdonly_ptr_(4);

void net_state_route_update(int ifindex, uint8_t family, uint8_t dst_len,
                            uint8_t protocol, uint8_t scope,
                            const uint8_t *dst, const uint8_t *gw,
                            bool has_gw, uint32_t priority,
                            bool defer_emit) _rdonly_ptr_(6) _access_(read_only, 7);
void net_state_route_remove(int ifindex, uint8_t family, uint8_t dst_len,
                            uint8_t protocol, uint8_t scope,
                            const uint8_t *dst, const uint8_t *gw,
                            bool has_gw, uint32_t priority,
                            bool defer_emit) _rdonly_ptr_(6) _access_(read_only, 7);

/* netlink.c */
int netlink_open(struct netlink_ctx *ctx) _wronly_ptr_(1);
void netlink_close(struct netlink_ctx *ctx) _rdwr_ptr_(1);
void netlink_start_dump(struct netlink_ctx *ctx) _rdwr_ptr_(1);
void netlink_dispatch(struct netlink_ctx *ctx) _rdwr_ptr_(1);

#endif /* _UEVENTD_NET_H */
