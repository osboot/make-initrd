// SPDX-License-Identifier: GPL-3.0-or-later

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <error.h>
#include <limits.h>

#include "rd/memory.h"
#include "rd/logging.h"

char *rd_strdup_or_die(const char *s)
{
	char *r = strdup(s);
	if (!r)
		rd_fatal("vasprintf: %m");
	return r;
}

void *rd_calloc_or_die(size_t nmemb, size_t size)
{
	void *r = calloc(nmemb, size);
	if (!r)
		rd_fatal("calloc: allocating %zu*%zu bytes: %m", nmemb, size);
	return r;
}

void *rd_malloc_or_die(size_t size)
{
	void *r = malloc(size);
	if (!r)
		rd_fatal("malloc: allocating %zu bytes: %m", size);
	return r;
}

char *rd_asprintf_or_die(const char *fmt, ...)
{
	char *ptr;
	va_list arg;

	va_start(arg, fmt);
	if (vasprintf(&ptr, fmt, arg) < 0)
		rd_fatal("vasprintf: %m");
	va_end(arg);

	return ptr;
}
