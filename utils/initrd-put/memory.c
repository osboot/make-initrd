// SPDX-License-Identifier: GPL-3.0-or-later

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sysexits.h>
#include <err.h>

#include "memory.h"

char *xstrdup(const char *str)
{
	char *x = strdup(str);
	if (!x)
		err(EX_OSERR, "strdup");
	return x;
}

char *xstrndup(const char *str, size_t size)
{
	char *x = strndup(str, size);
	if (!x)
		err(EX_OSERR, "strndup");
	return x;
}

void *xcalloc(size_t nmemb, size_t size)
{
	void *x = calloc(nmemb, size);

	if (!x)
		err(EX_OSERR, "calloc: allocating %lu*%lu bytes",
		    (unsigned long) nmemb, (unsigned long) size);
	return x;
}

void *xrealloc(void *ptr, size_t nmemb, size_t elem_size)
{
	if (nmemb && ULONG_MAX / nmemb < elem_size)
		errx(EX_OSERR, "realloc: nmemb*size > ULONG_MAX");

	void *x = realloc(ptr, nmemb * elem_size);

	if (!x)
		err(EX_OSERR, "realloc: allocating %lu*%lu bytes",
		    (unsigned long) nmemb, (unsigned long) elem_size);
	return x;
}

