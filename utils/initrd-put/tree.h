// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef __INITRD_PUT_TREE_H__
#define __INITRD_PUT_TREE_H__

#include "queue.h"

bool is_path_added(const char *path) __attribute__((__nonnull__ (1)));
bool tree_add_file(struct file *file) __attribute__((__nonnull__ (1)));
void tree_walk(void (*handler)(struct file *)) __attribute__((__nonnull__ (1)));
void tree_destroy(void);

#endif // __INITRD_PUT_TREE_H__
