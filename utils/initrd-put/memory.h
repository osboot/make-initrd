// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef __INITRD_PUT_MEMORY_H__
#define __INITRD_PUT_MEMORY_H__

void *xrealloc(void *ptr, size_t nmemb, size_t elem_size);
void *xcalloc(size_t nmemb, size_t size);
char *xstrdup(const char *s) __attribute__((__nonnull__ (1)));
char *xstrndup(const char *s, size_t n) __attribute__((__nonnull__ (1)));

#endif // __INITRD_PUT_MEMORY_H__

