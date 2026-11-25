// SPDX-License-Identifier: GPL-3.0-or-later

#include "config.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <poll.h>
#include <errno.h>

#include "rd/logging.h"
#include "rd/uevent.h"
#include "ueventd-net.h"

#define DEFAULT_LOGFILE "/var/log/ueventd-net.log"
#define QUEUE_NAME "network"

struct rd_uevent_queue event_queue;

static volatile sig_atomic_t running = 1;

static void handle_signal(int sig _maybe_unused_)
{
	running = 0;
}

static void setup_signals(void)
{
	struct sigaction sa;

	sa.sa_handler = handle_signal;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;

	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);
}

int main(int argc, char **argv)
{
	int c;
	int loglevel = LOG_INFO;
	struct netlink_ctx nl;

	while ((c = getopt(argc, argv, "hl:")) != -1) {
		switch (c) {
			case 'h':
				fprintf(stderr, "Usage: %s [-l loglevel]\n",
				        program_invocation_short_name);
				exit(EXIT_SUCCESS);
			case 'l':
				loglevel = rd_logging_level(optarg);
				break;
			default:
				exit(EXIT_FAILURE);
		}
	}

	rd_log_setup_stderr(rd_log_resolve_file("UEVENTD_NET_LOGFILE", DEFAULT_LOGFILE));
	rd_logging_init(fileno(stderr), loglevel, program_invocation_short_name);

	rd_info("starting server ...");

	setup_signals();
	rd_uevent_queue_open(&event_queue, rd_getenv_or_die("filterdir"), QUEUE_NAME);

	net_state_init();
	netlink_open(&nl);
	netlink_start_dump(&nl);

	struct pollfd fds[1] = {
		{
			.fd = nl.fd,
			.events = POLLIN,
		},
	};

	while (running) {
		int ready;

		errno = 0;
		ready = poll(fds, 1, -1);
		if (ready < 0) {
			if (errno == EINTR)
				continue;
			rd_fatal("poll: %m");
		}

		if (fds[0].revents & POLLIN)
			netlink_dispatch(&nl);
	}

	rd_info("stopping server ...");

	netlink_close(&nl);
	net_state_cleanup();
	rd_uevent_queue_close(&event_queue);
	rd_logging_close();

	return EXIT_SUCCESS;
}
