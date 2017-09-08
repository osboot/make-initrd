#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <error.h>
#include <limits.h>

#include "container.h"

void *
xmalloc(size_t size)
{
	void *r = malloc(size);

	if (!r)
		error(EXIT_FAILURE, errno, "malloc: allocating %lu bytes",
		      (unsigned long) size);
	return r;
}

void *
xcalloc(size_t nmemb, size_t size)
{
	void *r = calloc(nmemb, size);

	if (!r)
		error(EXIT_FAILURE, errno, "calloc: allocating %lu*%lu bytes",
		      (unsigned long) nmemb, (unsigned long) size);
	return r;
}

void *
xrealloc(void *ptr, size_t nmemb, size_t elem_size)
{
	if (nmemb && ULONG_MAX / nmemb < elem_size)
		error(EXIT_FAILURE, 0, "realloc: nmemb*size > ULONG_MAX");

	size_t size = nmemb * elem_size;
	void *r     = realloc(ptr, size);

	if (!r)
		error(EXIT_FAILURE, errno, "realloc: allocating %lu*%lu bytes",
		      (unsigned long) nmemb, (unsigned long) elem_size);
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

char *__attribute__((__format__(__printf__, 2, 3)))
__attribute__((__nonnull__(2)))
xasprintf(char **ptr, const char *fmt, ...)
{
	va_list arg;

	va_start(arg, fmt);
	if (vasprintf(ptr, fmt, arg) < 0)
		error(EXIT_FAILURE, errno, "vasprintf");
	va_end(arg);

	return *ptr;
}

void *
xfree(void *ptr)
{
	if (ptr)
		free(ptr);
	return NULL;
}
