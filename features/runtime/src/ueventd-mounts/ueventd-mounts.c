// SPDX-License-Identifier: GPL-3.0-or-later

#include "config.h"

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <poll.h>
#include <errno.h>

#include "rd/memory.h"
#include "rd/logging.h"
#include "rd/uevent.h"
#include "list.h"
#include "ueventd-mounts.h"
#include "runtime.h"

#define DEFAULT_LOGFILE "/var/log/ueventd-mounts.log"
#define QUEUE_NAME "mount"

static struct rd_uevent_queue event_queue;

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
	rd_uevent_emit(&event_queue, m->ent_path,
	               "ACTION='%s'\n"
	               "SOURCE='%s'\n"
	               "TARGET='%s'\n"
	               "FSTYPE='%s'\n"
	               "OPTIONS='%s'\n",
	               action, m->ent_name, m->ent_path, m->ent_type, m->ent_opts);
}

static struct mount_backend *backends[] = {
	&fanotify_backend,
	&procfs_backend,
};

static struct mount_backend *backend_select(void)
{
	for (size_t i = 0; i < ARRAY_SIZE(backends); i++) {
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

static void backend_dispatch(void *data)
{
	struct mount_backend *b = data;
	b->dispatch(b);
}

int ueventd_mounts_main(int argc, char **argv)
{
	rd_uevent_source_init(&event_queue, argc, argv, QUEUE_NAME,
	                      "UEVENTD_MOUNTS_LOGFILE", DEFAULT_LOGFILE);

	struct mount_backend *b = backend_select();

	rd_uevent_source_loop(b->fd, b->poll_events, backend_dispatch, b);

	b->cleanup(b);
	rd_uevent_source_close(&event_queue);
	return EXIT_SUCCESS;
}
