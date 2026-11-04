// SPDX-License-Identifier: GPL-3.0-or-later

#include "config.h"

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <mntent.h>
#include <errno.h>

#include "cleanup.h"
#include "rd/logging.h"
#include "ueventd-mounts.h"

#define MOUNT_PROC "/proc/self/mounts"

static void procfs_reset_state(void)
{
	struct mounts *m;
	list_for_each_entry(m, &mounts, list) {
		m->ent_flag = MOUNT_ENTRY_OLD;
	}
}

static void procfs_read_mounts(struct list_head *new_entries)
{
	struct mntent *mnt;
	FILE *fp __free(fclose) = fopen(MOUNT_PROC, "r");

	if (!fp)
		rd_fatal("fopen: %s: %m", MOUNT_PROC);

	while ((mnt = getmntent(fp))) {
		struct mounts *m = mount_find(0, mnt->mnt_dir, mnt->mnt_fsname, mnt->mnt_type);
		if (m) {
			m->ent_flag = MOUNT_ENTRY_CUR;
			continue;
		}

		m = mount_new(0, mnt->mnt_fsname, mnt->mnt_dir, mnt->mnt_type, mnt->mnt_opts);
		m->ent_flag = MOUNT_ENTRY_NEW;
		list_add_tail(&m->list, new_entries);
	}
}

static void procfs_diff_mounts(struct list_head *new_entries)
{
	struct mounts *pos, *tmp;

	list_for_each_entry_safe(pos, tmp, &mounts, list) {
		if (pos->ent_flag == MOUNT_ENTRY_OLD)
			mount_detach(pos);
	}

	list_for_each_entry_safe(pos, tmp, new_entries, list) {
		list_del(&pos->list);
		mount_attach(pos);
	}
}

static int procfs_init_backend(struct mount_backend *b)
{
	LIST_HEAD(new_entries);
	procfs_read_mounts(&new_entries);

	struct mounts *pos, *tmp;
	list_for_each_entry_safe(pos, tmp, &new_entries, list) {
		list_del(&pos->list);
		pos->ent_flag = MOUNT_ENTRY_CUR;
		list_add_tail(&pos->list, &mounts);
	}

	int fd = open(MOUNT_PROC, O_RDONLY | O_LARGEFILE | O_CLOEXEC);
	if (fd < 0) {
		rd_fatal("open: %s: %m", MOUNT_PROC);
		return -1;
	}

	b->fd = fd;
	b->poll_events = POLLERR | POLLPRI;
	return 0;
}

static void procfs_dispatch(struct mount_backend *b)
{
	(void) b;
	LIST_HEAD(new_entries);

	procfs_reset_state();
	procfs_read_mounts(&new_entries);
	procfs_diff_mounts(&new_entries);
}

static void procfs_cleanup(struct mount_backend *b)
{
	if (b->fd >= 0) {
		close(b->fd);
		b->fd = -1;
	}
}

struct mount_backend procfs_backend = {
	.name = "procfs",
	.init = procfs_init_backend,
	.dispatch = procfs_dispatch,
	.cleanup = procfs_cleanup,
};
