/* SPDX-License-Identifier: GPL-2.0-or-later */

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <error.h>
#include <limits.h>

#include "ueventd.h"

void *xcalloc(size_t nmemb, size_t size)
{
	void *r = calloc(nmemb, size);
	if (!r)
		rd_fatal("calloc: allocating %lu*%lu bytes: %m",
		         (unsigned long) nmemb, (unsigned long) size);
	return r;
}

char *xasprintf(char **ptr, const char *fmt, ...)
{
	va_list arg;

	va_start(arg, fmt);
	if (vasprintf(ptr, fmt, arg) < 0)
		rd_fatal("vasprintf: %m");
	va_end(arg);

	return *ptr;
}
