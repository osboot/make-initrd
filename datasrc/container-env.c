#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <ctype.h>
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <error.h>

#include "container.h"

void
load_environ(struct mapfile *envs)
{
	size_t i;
	char *nline, *a, *s, *eq;

	i = 1;
	a = envs->map;

	while (a && a[0]) {
		nline = strchr(a, '\n');

		while (a && isspace(a[0]))
			a++;

		if (a[0] == '#') {
			a = (nline) ? nline + 1 : NULL;
			i++;
			continue;
		}

		if (!nline)
			nline = a + strlen(a);

		s = strndup(a, (size_t)(nline - a));
		a = nline + 1;

		if ((eq = strchr(s, '=')) && s < eq)
			putenv(s);
		else
			xfree(s);

		i++;
	}

	close_map(envs);
}

int
    __attribute__((format(printf, 2, 3)))
    setenvf(const char *name, const char *fmt, ...)
{
	int rc;
	char *value = NULL;
	va_list ap;

	va_start(ap, fmt);

	if (vasprintf(&value, fmt, ap) < 0) {
		error(EXIT_SUCCESS, errno, "vasprintf");
		return -1;
	}

	va_end(ap);

	rc = setenv(name, value, 1);
	xfree(value);

	return rc;
}
