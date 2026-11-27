// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef MKRD_INITRD_PUT_TREE_H
#define MKRD_INITRD_PUT_TREE_H

#include "queue.h"

bool is_path_added(const char *path) _nonnull_(1);
bool tree_add_file(struct file *file) _nonnull_(1);
size_t tree_get_nr(void);
void tree_walk(void (*handler)(struct file *)) _nonnull_(1);
void tree_destroy(void);

#endif // MKRD_INITRD_PUT_TREE_H
