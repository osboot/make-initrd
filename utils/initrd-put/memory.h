// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef MKRD_INITRD_PUT_MEMORY_H
#define MKRD_INITRD_PUT_MEMORY_H

#include "compiler.h"

void *xrealloc(void *ptr, size_t nmemb, size_t elem_size) _alloc_(2, 3) _returns_nonnull_ _must_check_;
void *xcalloc(size_t nmemb, size_t size) _alloc_(1, 2) _returns_nonnull_ _must_check_;
char *xstrdup(const char *s) _nonnull_(1) _returns_nonnull_ _must_check_;
char *xstrndup(const char *s, size_t n) _rdonly_ptr_(1, 2) _returns_nonnull_ _must_check_;

#endif // MKRD_INITRD_PUT_MEMORY_H

