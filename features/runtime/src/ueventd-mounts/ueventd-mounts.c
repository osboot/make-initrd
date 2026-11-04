// SPDX-License-Identifier: GPL-3.0-or-later

#include "config.h"

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <poll.h>
#include <time.h>
#include <errno.h>

#include "cleanup.h"
#include "rd/memory.h"
#include "rd/logging.h"
#include "rd/list.h"
#include "ueventd-mounts.h"

#define DEFAULT_LOGFILE "/var/log/ueventd-mounts.log"

#define QUEUE_TMPNAME ".tmp"
#define QUEUE_NAME "mount"
#define EVENT_TEMP_NAME "." QUEUE_NAME ".event.new"

static char *filter_dir;
static int queue_dirfd;
static int temp_dirfd;

LIST_HEAD(mounts);

struct mounts *mount_new(uint64_t mnt_id, const char *source,
                         const char *target, const char *fstype,
                         const char *opts)
{
	struct mounts *m = rd_calloc_or_die(1, sizeof(*m));
	m->mnt_id = mnt_id;
	m->ent_name = rd_strdup_or_die(source ? source : "");
	m->ent_path = rd_strdup_or_die(target ? target : "");
	m->ent_type = rd_strdup_or_die(fstype ? fstype : "");
	m->ent_opts = rd_strdup_or_die(opts ? opts : "");
	m->ent_flag = MOUNT_ENTRY_CUR;
	return m;
}

void free_mount(struct mounts *m)
{
	if (!m)
		return;
	free(m->ent_name);
	free(m->ent_path);
	free(m->ent_type);
	free(m->ent_opts);
	free(m);
}

struct mounts *mount_find(uint64_t mnt_id, const char *target,
                          const char *source, const char *fstype)
{
	struct mounts *m;
	list_for_each_entry(m, &mounts, list) {
		if (mnt_id && m->mnt_id) {
			if (m->mnt_id == mnt_id)
				return m;
		} else if (target && source && fstype) {
			if (!strcmp(m->ent_path, target) &&
			    !strcmp(m->ent_name, source) &&
			    !strcmp(m->ent_type, fstype))
				return m;
		}
	}
	return NULL;
}

void mount_emit(const char *action, struct mounts *m)
{
	rd_info("%s: %s", action, m->ent_path);
	create_event(action, m);
}

void mount_attach(struct mounts *m)
{
	list_add_tail(&m->list, &mounts);
	mount_emit("mount", m);
}

void mount_detach(struct mounts *m)
{
	mount_emit("umount", m);
	list_del(&m->list);
	free_mount(m);
}

void create_event(const char *action, struct mounts *m)
{
	char *p;
	struct timespec tp = { 0 };

	clock_gettime(CLOCK_BOOTTIME, &tp);

	int fd __free(close) = openat(temp_dirfd, EVENT_TEMP_NAME,
	                              O_WRONLY | O_CREAT | O_EXCL,
	                              S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (fd < 0)
		rd_fatal("open: %s/" QUEUE_TMPNAME "/%s: %m", filter_dir, EVENT_TEMP_NAME);

	dprintf(fd,
	        "ACTION='%s'\n" "SOURCE='%s'\n" "TARGET='%s'\n" "FSTYPE='%s'\n" "OPTIONS='%s'\n",
	        action, m->ent_name, m->ent_path, m->ent_type, m->ent_opts);

	char *fname __free(free) = rd_asprintf_or_die(QUEUE_NAME ".%s.%06lu.%012lu", m->ent_path,
	                                              (unsigned long) tp.tv_sec, tp.tv_nsec);

	for (p = fname; *p != '\0'; p++) {
		if (*p == '/')
			*p = '_';
	}

	if (renameat(temp_dirfd, EVENT_TEMP_NAME, queue_dirfd, fname) < 0)
		rd_fatal("rename `%s/" QUEUE_TMPNAME "/%s' -> `%s/%s': %m",
		         filter_dir, EVENT_TEMP_NAME,
		         filter_dir, fname);
}

static char *get_param_dir(const char *name)
{
	char *value = getenv(name);
	if (!value)
		rd_fatal("please set `%s' env variable", name);
	return value;
}

static struct mount_backend *backends[] = {
	&fanotify_backend,
	&procfs_backend,
};

static struct mount_backend *backend_select(void)
{
	for (size_t i = 0; i < sizeof(backends) / sizeof(backends[0]); i++) {
		struct mount_backend *b = backends[i];
		if (b->init(b) == 0) {
			rd_info("using %s mount monitoring backend", b->name);
			return b;
		}
		rd_info("%s backend not available; falling back", b->name);
	}
	rd_fatal("no mount monitoring backend available");
	return NULL;
}

int main(int argc, char **argv)
{
	int c;
	int loglevel = LOG_INFO;

	while ((c = getopt(argc, argv, "hl:")) != -1) {
		switch (c) {
			case 'h':
				fprintf(stderr, "Usage: %s [-l loglevel]\n",
				        program_invocation_short_name);
				exit(EXIT_SUCCESS);
				break;
			case 'l':
				loglevel = rd_logging_level(optarg);
				break;
			default:
				exit(EXIT_FAILURE);
				break;
		}
	}

	const char *logfile = getenv("UEVENTD_MOUNTS_LOGFILE");
	if (!logfile || !*logfile)
		logfile = getenv("LOGFILE");
	if (!logfile || !*logfile)
		logfile = DEFAULT_LOGFILE;

	rd_log_setup_stderr(logfile);
	rd_logging_init(fileno(stderr), loglevel, program_invocation_short_name);

	rd_info("starting server ...");

	filter_dir = get_param_dir("filterdir");

	if (chdir(filter_dir) < 0)
		rd_fatal("chdir: %s: %m", filter_dir);

	if (chdir(QUEUE_NAME) < 0)
		rd_fatal("chdir: %s/" QUEUE_NAME ": %m", filter_dir);

	if ((queue_dirfd = open(".", O_PATH | O_DIRECTORY | O_CLOEXEC)) < 0)
		rd_fatal("open: %s/" QUEUE_NAME ": %m", filter_dir);

	if ((temp_dirfd = openat(queue_dirfd, QUEUE_TMPNAME, O_PATH | O_DIRECTORY | O_CLOEXEC)) < 0)
		rd_fatal("open: %s/" QUEUE_TMPNAME ": %m", filter_dir);

	struct mount_backend *b = backend_select();

	struct pollfd fds[1] = {
		{
			.fd = b->fd,
			.events = b->poll_events,
		},
	};

	while (1) {
		int ready;

		errno = 0;
		if ((ready = poll(fds, 1, -1)) < 0) {
			if (errno == EINTR)
				continue;
			rd_fatal("poll: %m");
		}

		if (fds[0].revents & b->poll_events)
			b->dispatch(b);
	}

	b->cleanup(b);
	return EXIT_SUCCESS;
}
