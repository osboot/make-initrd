// SPDX-License-Identifier: GPL-3.0-or-later

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <poll.h>
#include <mntent.h>
#include <time.h>
#include <errno.h>

#include "rd/memory.h"
#include "rd/logging.h"
#include "rd/list.h"

#define MOUNT_PROC "/proc/self/mounts"
#define DEFAULT_LOGFILE "/var/log/ueventd-mounts.log"

#define QUEUE_TMPNAME ".tmp"
#define QUEUE_NAME "mount"
#define EVENT_TEMP_NAME "." QUEUE_NAME ".event.new"

enum mount_entry_t {
	MOUNT_ENTRY_OLD,
	MOUNT_ENTRY_NEW,
	MOUNT_ENTRY_CUR,
};

struct mounts {
	struct list_head list;
	enum mount_entry_t ent_flag;
	char *ent_name;
	char *ent_path;
	char *ent_type;
	char *ent_opts;
};

static void reset_state(void);
static void read_mounts(void);
static void diff_mounts(void);
static struct mounts *find_mounts(const struct mntent *mnt)     __attribute__((nonnull(1)));
static void create_event(const char *action, struct mounts *m)  __attribute__((nonnull(1,2)));
static inline char *get_param_dir(const char *name)             __attribute__((nonnull(1)));

static char *filter_dir;
static int queue_dirfd;
static int temp_dirfd;

LIST_HEAD(mounts);

void reset_state(void)
{
	struct mounts *mount;
	list_for_each_entry(mount, &mounts, list) {
		mount->ent_flag = MOUNT_ENTRY_OLD;
	}
}

struct mounts *find_mounts(const struct mntent *mnt)
{
	struct mounts *mount;
	list_for_each_entry(mount, &mounts, list) {
		if (!strcmp(mount->ent_path, mnt->mnt_dir) &&
		    !strcmp(mount->ent_name, mnt->mnt_fsname) &&
		    !strcmp(mount->ent_type, mnt->mnt_type))
			return mount;
	}
	return NULL;
}

void read_mounts(void)
{
	struct mntent *mnt;
	FILE *fp;

	if (!(fp = fopen(MOUNT_PROC, "r")))
		rd_fatal("fopen: %s: %m", MOUNT_PROC);

	while ((mnt = getmntent(fp))) {
		struct mounts *mount = find_mounts(mnt);
		if (mount) {
			mount->ent_flag = MOUNT_ENTRY_CUR;
			continue;
		}

		mount = rd_calloc_or_die(1, sizeof(*mount));

		mount->ent_name = rd_strdup_or_die(mnt->mnt_fsname);
		mount->ent_path = rd_strdup_or_die(mnt->mnt_dir);
		mount->ent_type = rd_strdup_or_die(mnt->mnt_type);
		mount->ent_opts = rd_strdup_or_die(mnt->mnt_opts);
		mount->ent_flag = MOUNT_ENTRY_NEW;

		list_add_tail(&mount->list, &mounts);
	}

	fclose(fp);
}

void create_event(const char *action, struct mounts *m)
{
	int fd;
	char *p, *fname;
	struct timespec tp = { 0 };

	clock_gettime(CLOCK_BOOTTIME, &tp);

	fd = openat(temp_dirfd, EVENT_TEMP_NAME,
	            O_WRONLY | O_CREAT | O_EXCL,
	            S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (fd < 0)
		rd_fatal("open: %s/" QUEUE_TMPNAME "/%s: %m", filter_dir, EVENT_TEMP_NAME);

	dprintf(fd,
	        "ACTION='%s'\n" "SOURCE='%s'\n" "TARGET='%s'\n" "FSTYPE='%s'\n" "OPTIONS='%s'\n",
	        action, m->ent_name, m->ent_path, m->ent_type, m->ent_opts);

	close(fd);

	fname = rd_asprintf_or_die(QUEUE_NAME ".%s.%06lu.%012lu", m->ent_path,
	                           (unsigned long) tp.tv_sec, tp.tv_nsec);

	for (p = fname; *p != '\0'; p++) {
		if (*p == '/')
			*p = '_';
	}

	if (renameat(temp_dirfd, EVENT_TEMP_NAME, queue_dirfd, fname) < 0)
		rd_fatal("rename `%s/" QUEUE_TMPNAME "/%s' -> `%s/%s': %m",
		         filter_dir, EVENT_TEMP_NAME,
		         filter_dir, fname);

	free(fname);
}

void diff_mounts(void)
{
	struct mounts *pos, *tmp;

	list_for_each_entry_safe(pos, tmp, &mounts, list) {
		switch (pos->ent_flag) {
			case MOUNT_ENTRY_NEW:
				rd_info("mount: %s", pos->ent_path);
				create_event("mount", pos);
				break;
			case MOUNT_ENTRY_OLD:
				rd_info("umount: %s", pos->ent_path);
				create_event("umount", pos);

				list_del(&pos->list);

				free(pos->ent_name);
				free(pos->ent_path);
				free(pos->ent_type);
				free(pos->ent_opts);
				free(pos);
				break;
			default:
		}
	}
}

char *get_param_dir(const char *name)
{
	char *value = getenv(name);
	if (!value)
		rd_fatal("please set `%s' env variable", name);
	return value;
}

int main(int argc, char **argv)
{
	int c, fd;
	struct pollfd fds[1];
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

	rd_log_setup_stderr(DEFAULT_LOGFILE);
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

	read_mounts();

	if ((fd = open(MOUNT_PROC, O_RDONLY | O_LARGEFILE | O_CLOEXEC)) < 0)
		rd_fatal("open: %s: %m", MOUNT_PROC);

	fds[0].fd = fd;
	fds[0].events = POLLERR | POLLPRI;

	while (1) {
		int ready;

		errno = 0;
		if ((ready = poll(fds, 1, -1)) < 0) {
			if (errno == EINTR)
				continue;
			rd_fatal("poll: %m");
		}

		if (fds[0].revents & POLLERR) {
			reset_state();
			read_mounts();
			diff_mounts();
		}
	}

	close(fd);

	return EXIT_SUCCESS;
}
