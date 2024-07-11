// SPDX-License-Identifier: GPL-3.0-or-later
#define _GNU_SOURCE

#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <err.h>

#include "tree.h"
#include "queue.h"
#include "enqueue.h"

extern int verbose;

bool is_shebang(char buf[LINE_MAX])
{
	return (buf[0] == '#' &&
	        buf[1] == '!');
}

int enqueue_shebang(const char *filename, char buf[LINE_MAX])
{
	char *p, *q;

	for (p = &buf[2]; *p && isspace(*p); p++);
	for (q = p; *q && (!isspace(*q)); q++);
	*q = '\0';

	if (verbose > 1)
		warnx("shell script '%s' uses the '%s' interpreter", filename, p);

	if (!is_path_added(p))
		enqueue_item(p, -1);

	return 0;
}
