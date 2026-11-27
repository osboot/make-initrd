// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef MKRD_INITRD_PUT_QUEUE_H
#define MKRD_INITRD_PUT_QUEUE_H

#include <sys/stat.h>
#include <sys/types.h>
#include <stdbool.h>

#include "compiler.h"

struct file {
	struct file *prev;
	struct file *next;
	struct stat stat;
	char *src;
	char *dst;
	char *symlink;
	struct file *hardlink_target;
	bool recursive;
	bool installed;
};

struct file *get_queue(size_t *nr) _access_(write_only, 1);
void free_file(void *ptr) _nonnull_(1);
struct file *enqueue_item(const char *str, ssize_t len) _nonnull_(1);
void dequeue_item(struct file *ptr);
bool is_path_queued(const char *path) _nonnull_(1);
void queue_destroy(void);

#endif // MKRD_INITRD_PUT_QUEUE_H

