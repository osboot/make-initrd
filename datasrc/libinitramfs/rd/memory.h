/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef __RD_MEMORY_H__
#define __RD_MEMORY_H__

#include <stdlib.h>

void *rd_calloc_or_die(size_t nmemb, size_t size)          __attribute__((alloc_size(1, 2)));
void *rd_malloc_or_die(size_t size)                        __attribute__((alloc_size(1)));
char *rd_asprintf_or_die(char **ptr, const char *fmt, ...) __attribute__((format(printf, 2, 3)));

#endif /* __RD_MEMORY_H__ */
