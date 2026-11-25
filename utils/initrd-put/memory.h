// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef __INITRD_PUT_MEMORY_H__
#define __INITRD_PUT_MEMORY_H__

#include "compiler.h"

void *xrealloc(void *ptr, size_t nmemb, size_t elem_size);
void *xcalloc(size_t nmemb, size_t size);
char *xstrdup(const char *s) _nonnull_(1);
char *xstrndup(const char *s, size_t n) _nonnull_(1);

#endif // __INITRD_PUT_MEMORY_H__

