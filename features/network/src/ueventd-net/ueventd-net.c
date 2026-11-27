// SPDX-License-Identifier: GPL-3.0-or-later

#include "config.h"

#include <unistd.h>
#include <stdlib.h>
#include <poll.h>

#include "rd/logging.h"
#include "rd/uevent.h"
#include "ueventd-net.h"

#define DEFAULT_LOGFILE "/var/log/ueventd-net.log"
#define QUEUE_NAME "network"

struct rd_uevent_queue event_queue;

static void dispatch_netlink(void *data)
{
	netlink_dispatch(data);
}

int main(int argc, char **argv)
{
	struct netlink_ctx nl;

	rd_uevent_source_init(&event_queue, argc, argv, QUEUE_NAME,
	                      "UEVENTD_NET_LOGFILE", DEFAULT_LOGFILE);

	net_state_init();
	netlink_open(&nl);
	netlink_start_dump(&nl);

	rd_uevent_source_loop(nl.fd, POLLIN, dispatch_netlink, &nl);

	netlink_close(&nl);
	net_state_cleanup();
	rd_uevent_source_close(&event_queue);

	return EXIT_SUCCESS;
}
