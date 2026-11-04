// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef _UEVENTD_MOUNTS_H
#define _UEVENTD_MOUNTS_H 1

#include <stdint.h>
#include <stdbool.h>
#include <poll.h>

#include "compiler.h"
#include "rd/list.h"

enum mount_entry_t {
	MOUNT_ENTRY_OLD,
	MOUNT_ENTRY_NEW,
	MOUNT_ENTRY_CUR,
};

struct mounts {
	struct list_head list;
	enum mount_entry_t ent_flag;
	uint64_t mnt_id;
	char *ent_name;
	char *ent_path;
	char *ent_type;
	char *ent_opts;
};

struct mount_backend {
	const char *name;
	int fd;
	short poll_events;
	int (*init)(struct mount_backend *b);
	void (*dispatch)(struct mount_backend *b);
	void (*cleanup)(struct mount_backend *b);
};

extern struct list_head mounts;

struct mounts *mount_new(uint64_t mnt_id, const char *source,
                         const char *target, const char *fstype,
                         const char *opts);
void free_mount(struct mounts *mount);
struct mounts *mount_find(uint64_t mnt_id, const char *target,
                          const char *source, const char *fstype);
void mount_emit(const char *action, struct mounts *m) __nonnull(1, 2);
void mount_attach(struct mounts *mount) __nonnull(1);
void mount_detach(struct mounts *mount) __nonnull(1);

void create_event(const char *action, struct mounts *m) __nonnull(1, 2);

extern struct mount_backend fanotify_backend;
extern struct mount_backend procfs_backend;

#endif /* _UEVENTD_MOUNTS_H */
