// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef __INITRD_PUT_TREE_H__
#define __INITRD_PUT_TREE_H__

#include "queue.h"

bool is_path_added(const char *path) __nonnull(1);
bool tree_add_file(struct file *file) __nonnull(1);
size_t tree_get_nr(void);
void tree_walk(void (*handler)(struct file *)) __nonnull(1);
void tree_destroy(void);

#endif // __INITRD_PUT_TREE_H__
