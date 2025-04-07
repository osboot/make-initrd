// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef __INITRD_PUT_QUEUE_H__
#define __INITRD_PUT_QUEUE_H__

#include <sys/stat.h>
#include <sys/types.h>
#include <stdbool.h>

struct file {
	struct file *prev;
	struct file *next;
	struct stat stat;
	char *src;
	char *dst;
	char *symlink;
	bool recursive;
	bool installed;
};

struct file *get_queue(size_t *nr);
void free_file(void *ptr) __attribute__((__nonnull__ (1)));
struct file *enqueue_item(const char *str, ssize_t len) __attribute__((__nonnull__ (1)));
void dequeue_item(struct file *ptr);

#endif // __INITRD_PUT_QUEUE_H__

