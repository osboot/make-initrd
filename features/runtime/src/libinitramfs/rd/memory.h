// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef __RD_MEMORY_H__
#define __RD_MEMORY_H__

#include <stdlib.h>

#include "compiler.h"

char *rd_strdup_or_die(const char *s)                      _nonnull_(1);
void *rd_calloc_or_die(size_t nmemb, size_t size)          __attribute__((alloc_size(1, 2),returns_nonnull)) _must_check_;
void *rd_malloc_or_die(size_t size)                        __attribute__((alloc_size(1),returns_nonnull)) _must_check_;
char *rd_asprintf_or_die(const char *fmt, ...)             _nonnull_(1) _printf_(1, 2) __attribute__((returns_nonnull))
                                                           _must_check_;

#endif /* __RD_MEMORY_H__ */
