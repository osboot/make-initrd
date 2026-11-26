// SPDX-License-Identifier: GPL-3.0-or-later

#include "config.h"

#include <sys/stat.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <inttypes.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>
#include <time.h>

#include "cleanup.h"
#include "rd/logging.h"
#include "rd/memory.h"
#include "rd/uevent.h"

#define QUEUE_TMPNAME ".tmp"

char *rd_getenv_or_die(const char *name)
{
	char *value = getenv(name);
	if (!value)
		rd_fatal("please set `%s' env variable", name);
	return value;
}

void rd_uevent_queue_open(struct rd_uevent_queue *q,
                          const char *filter_dir,
                          const char *queue_name)
{
	char *queue_path _free_(free) = rd_asprintf_or_die("%s/%s", filter_dir, queue_name);

	q->filter_dir = filter_dir;
	q->name = queue_name;
	q->seq = 0;

	if (mkdir(queue_path, 0755) < 0 && errno != EEXIST)
		rd_fatal("mkdir: %s: %m", queue_path);

	q->queue_dirfd = open(queue_path, O_PATH | O_DIRECTORY | O_CLOEXEC);
	if (q->queue_dirfd < 0)
		rd_fatal("open: %s: %m", queue_path);

	if (mkdirat(q->queue_dirfd, QUEUE_TMPNAME, 0755) < 0 && errno != EEXIST)
		rd_fatal("mkdir: %s/" QUEUE_TMPNAME ": %m", queue_path);

	q->temp_dirfd = openat(q->queue_dirfd, QUEUE_TMPNAME, O_PATH | O_DIRECTORY | O_CLOEXEC);
	if (q->temp_dirfd < 0)
		rd_fatal("open: %s/" QUEUE_TMPNAME ": %m", queue_path);
}

void rd_uevent_queue_close(struct rd_uevent_queue *q)
{
	if (q->temp_dirfd >= 0) {
		close(q->temp_dirfd);
		q->temp_dirfd = -1;
	}
	if (q->queue_dirfd >= 0) {
		close(q->queue_dirfd);
		q->queue_dirfd = -1;
	}
}

void rd_uevent_emit(struct rd_uevent_queue *q,
                    const char *event_name,
                    const char *fmt, ...)
{
	char *p;
	struct timespec tp = { 0 };
	va_list ap;

	clock_gettime(CLOCK_BOOTTIME, &tp);

	char *tmpname _free_(free) = rd_asprintf_or_die(".%s.event.new", q->name);

	int fd _free_(close) = openat(q->temp_dirfd, tmpname,
	                              O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC,
	                              S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (fd < 0)
		rd_fatal("open: %s/%s/" QUEUE_TMPNAME "/%s: %m",
		         q->filter_dir, q->name, tmpname);

	va_start(ap, fmt);
	if (vdprintf(fd, fmt, ap) < 0)
		rd_fatal("vdprintf: %s/%s/" QUEUE_TMPNAME "/%s: %m",
		         q->filter_dir, q->name, tmpname);
	va_end(ap);

	char *fname _free_(free) = rd_asprintf_or_die("%s.%s.%06lu.%012lu.%06" PRIu64,
	                                              q->name, event_name,
	                                              (unsigned long) tp.tv_sec,
	                                              (unsigned long) tp.tv_nsec,
	                                              q->seq++);

	for (p = fname; *p != '\0'; p++) {
		if (*p == '/')
			*p = '_';
	}

	if (renameat(q->temp_dirfd, tmpname, q->queue_dirfd, fname) < 0)
		rd_fatal("rename `%s/%s/" QUEUE_TMPNAME "/%s' -> `%s/%s/%s': %m",
		         q->filter_dir, q->name, tmpname,
		         q->filter_dir, q->name, fname);
}

static volatile sig_atomic_t source_running = 1;

static void handle_source_signal(int sig _maybe_unused_)
{
	source_running = 0;
}

static void setup_source_signals(void)
{
	struct sigaction sa = {
		.sa_handler = handle_source_signal,
		.sa_flags = 0,
	};

	sigemptyset(&sa.sa_mask);
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);
}

void rd_uevent_source_init(struct rd_uevent_queue *q,
                           int argc, char **argv,
                           const char *queue_name,
                           const char *logfile_env,
                           const char *default_logfile)
{
	int c;
	int loglevel = LOG_INFO;

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

	rd_log_setup_stderr(rd_log_resolve_file(logfile_env, default_logfile));
	rd_logging_init(fileno(stderr), loglevel, program_invocation_short_name);

	rd_info("starting server ...");

	setup_source_signals();
	rd_uevent_queue_open(q, rd_getenv_or_die("filterdir"), queue_name);
}

void rd_uevent_source_loop(int fd, short poll_events,
                           void (*dispatch)(void *data),
                           void *data)
{
	struct pollfd fds[1] = {
		{
			.fd = fd,
			.events = poll_events,
		},
	};

	source_running = 1;

	while (source_running) {
		int ready;

		errno = 0;
		ready = poll(fds, 1, -1);
		if (ready < 0) {
			if (errno == EINTR)
				continue;
			rd_fatal("poll: %m");
		}

		if (fds[0].revents & poll_events)
			dispatch(data);
	}

	rd_info("stopping server ...");
}

void rd_uevent_source_close(struct rd_uevent_queue *q)
{
	rd_uevent_queue_close(q);
	rd_logging_close();
}
