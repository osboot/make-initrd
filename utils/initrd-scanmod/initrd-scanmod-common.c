// SPDX-License-Identifier: GPL-3.0-or-later
#define _GNU_SOURCE

#include "config.h"

#include <sys/types.h>
#include <sys/param.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <err.h>

#include "initrd-scanmod.h"

void *
xmalloc(size_t size)
{
	void *r = malloc(size);

	if (!r)
		err(EXIT_FAILURE, "malloc: allocating %lu bytes",
		    (unsigned long) size);
	return r;
}

void *
xrealloc(void *ptr, size_t nmemb, size_t elem_size)
{
	if (nmemb && ULONG_MAX / nmemb < elem_size)
		errx(EXIT_FAILURE, "realloc: nmemb*size > ULONG_MAX");

	size_t size = nmemb * elem_size;
	void *r     = realloc(ptr, size);

	if (!r)
		err(EXIT_FAILURE, "realloc: allocating %lu*%lu bytes",
		    (unsigned long) nmemb, (unsigned long) elem_size);
	return r;
}

void *
xcalloc(size_t nmemb, size_t size)
{
	void *r = calloc(nmemb, size);

	if (!r)
		err(EXIT_FAILURE, "calloc: allocating %lu*%lu bytes",
		    (unsigned long) nmemb, (unsigned long) size);
	return r;
}

char *
xstrdup(const char *s)
{
	size_t len = strlen(s);
	char *r    = xmalloc(len + 1);

	memcpy(r, s, len + 1);
	return r;
}

int __attribute__((__format__(__printf__, 2, 3)))
__attribute__((__nonnull__(2)))
xasprintf(char **ptr, const char *fmt, ...)
{
	int ret;
	va_list arg;

	va_start(arg, fmt);
	if ((ret = vasprintf(ptr, fmt, arg)) < 0)
		err(EXIT_FAILURE, "vasprintf");
	va_end(arg);

	return ret;
}

void *
xfree(void *ptr)
{
	if (ptr)
		free(ptr);
	return NULL;
}
