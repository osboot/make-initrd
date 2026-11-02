// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef __RD_MEMORY_H__
#define __RD_MEMORY_H__

#include <stdlib.h>

#include "compiler.h"

char *rd_strdup_or_die(const char *s)                      __nonnull(1);
void *rd_calloc_or_die(size_t nmemb, size_t size)          __attribute__((alloc_size(1, 2),returns_nonnull)) __must_check;
void *rd_malloc_or_die(size_t size)                        __attribute__((alloc_size(1),returns_nonnull)) __must_check;
char *rd_asprintf_or_die(const char *fmt, ...)             __nonnull(1) __printf(1, 2) __attribute__((returns_nonnull))
                                                           __must_check;

#endif /* __RD_MEMORY_H__ */
