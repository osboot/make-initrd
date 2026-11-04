// SPDX-License-Identifier: GPL-3.0-or-later

#include "config.h"

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <errno.h>

#include "cleanup.h"
#include "rd/memory.h"
#include "rd/logging.h"
#include "ueventd-mounts.h"
#include "mount-compat.h"

static struct mounts *statmount_alloc(uint64_t mnt_id)
{
	char buf[4096];
	struct rd_statmount *sm = (struct rd_statmount *) buf;
	struct rd_mnt_id_req req = {
		.size = MNT_ID_REQ_SIZE_VER0,
		.mnt_id = mnt_id,
		.param = STATMOUNT_MNT_POINT | STATMOUNT_FS_TYPE |
		         STATMOUNT_MNT_OPTS | STATMOUNT_SB_SOURCE,
	};

	if (sys_statmount(&req, sm, sizeof(buf), 0) < 0)
		return NULL;

	const char *point = (sm->mask & STATMOUNT_MNT_POINT) ? (sm->str + sm->mnt_point) : "";
	const char *type = (sm->mask & STATMOUNT_FS_TYPE) ? (sm->str + sm->fs_type) : "";
	const char *source = (sm->mask & STATMOUNT_SB_SOURCE) ? (sm->str + sm->sb_source) : "";
	const char *opts = (sm->mask & STATMOUNT_MNT_OPTS) ? (sm->str + sm->mnt_opts) : "";

	return mount_new(mnt_id, source, point, type, opts);
}

static int fanotify_read_mounts(void)
{
	uint64_t mnt_ids[256];
	struct rd_mnt_id_req req = {
		.size = MNT_ID_REQ_SIZE_VER0,
		.mnt_id = LSMT_ROOT,
		.param = 0,
	};

	while (1) {
		long nr = sys_listmount(&req, mnt_ids, sizeof(mnt_ids) / sizeof(mnt_ids[0]), 0);
		if (nr < 0)
			return -1;
		if (nr == 0)
			break;

		for (long i = 0; i < nr; i++) {
			struct mounts *m = mount_find(mnt_ids[i], NULL, NULL, NULL);
			if (m)
				continue;

			m = statmount_alloc(mnt_ids[i]);
			if (!m)
				continue;

			list_add_tail(&m->list, &mounts);
		}

		if (nr < (long) (sizeof(mnt_ids) / sizeof(mnt_ids[0])))
			break;

		req.param = mnt_ids[nr - 1];
	}

	return 0;
}

static int fanotify_init_backend(struct mount_backend *b)
{
	int fd = fanotify_init(FAN_CLASS_NOTIF | FAN_REPORT_MNT | FAN_NONBLOCK, O_RDONLY | O_CLOEXEC);
	if (fd < 0)
		return -1;

	int mntns_fd = -1;
	int pidfd = sys_pidfd_open(getpid(), 0);
	if (pidfd >= 0) {
		mntns_fd = ioctl(pidfd, PIDFD_GET_MNT_NAMESPACE);
		close(pidfd);
	}

	if (mntns_fd < 0)
		mntns_fd = open("/proc/self/ns/mnt", O_RDONLY | O_CLOEXEC);

	if (mntns_fd < 0) {
		close(fd);
		return -1;
	}

	int ret = fanotify_mark(fd, FAN_MARK_ADD | FAN_MARK_MNTNS,
	                        FAN_MNT_ATTACH | FAN_MNT_DETACH,
	                        mntns_fd, NULL);
	close(mntns_fd);

	if (ret < 0) {
		close(fd);
		return -1;
	}

	if (fanotify_read_mounts() < 0) {
		close(fd);
		return -1;
	}

	b->fd = fd;
	b->poll_events = POLLIN;
	return 0;
}

static void fanotify_dispatch(struct mount_backend *b)
{
	char buf[4096] __attribute__((aligned(__alignof__(struct fanotify_event_metadata))));

	while (1) {
		ssize_t len = read(b->fd, buf, sizeof(buf));
		if (len < 0) {
			if (errno == EINTR)
				continue;
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break;
			rd_fatal("read fanotify: %m");
		}
		if (len == 0)
			break;

		const struct fanotify_event_metadata *meta = (struct fanotify_event_metadata *) buf;
		while (FAN_EVENT_OK(meta, len)) {
			if (meta->vers != FANOTIFY_METADATA_VERSION)
				rd_fatal("unsupported fanotify metadata version");

			if (meta->fd != FAN_NOFD)
				close(meta->fd);

			const char *info_ptr = (const char *) (meta + 1);
			const char *info_end = ((const char *) meta) + meta->event_len;

			while (info_ptr + sizeof(struct fanotify_event_info_header) <= info_end) {
				const struct fanotify_event_info_header *hdr =
					(const struct fanotify_event_info_header *) info_ptr;

				if (hdr->len < sizeof(*hdr) || info_ptr + hdr->len > info_end)
					break;

				if (hdr->info_type == FAN_EVENT_INFO_TYPE_MNT) {
					const struct fanotify_event_info_mnt *imnt =
						(const struct fanotify_event_info_mnt *) hdr;
					uint64_t mnt_id = imnt->mnt_id;

					if (meta->mask & FAN_MNT_DETACH) {
						struct mounts *m = mount_find(mnt_id, NULL, NULL, NULL);
						if (m)
							mount_detach(m);
					}

					if (meta->mask & FAN_MNT_ATTACH) {
						struct mounts *m = mount_find(mnt_id, NULL, NULL, NULL);
						if (m)
							mount_detach(m);

						m = statmount_alloc(mnt_id);
						if (m)
							mount_attach(m);
					}
				}

				info_ptr += hdr->len;
			}

			meta = FAN_EVENT_NEXT(meta, len);
		}
	}
}

static void fanotify_cleanup(struct mount_backend *b)
{
	if (b->fd >= 0) {
		close(b->fd);
		b->fd = -1;
	}
}

struct mount_backend fanotify_backend = {
	.name = "fanotify",
	.init = fanotify_init_backend,
	.dispatch = fanotify_dispatch,
	.cleanup = fanotify_cleanup,
};
