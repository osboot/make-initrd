// SPDX-License-Identifier: GPL-3.0-or-later

#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sysexits.h>
#include <err.h>
#include <regex.h>

#include "memory.h"
#include "queue.h"

extern int verbose;
extern char *prefix;
extern regex_t *exclude_match;
extern size_t exclude_match_nr;

static struct file *inqueue = NULL;
static size_t queue_nr = 0;

struct file *get_queue(size_t *nr)
{
	if (nr)
		*nr = queue_nr;

	return inqueue;
}

struct file *enqueue_item(const char *str, ssize_t len)
{
	struct file *new;

	if (exclude_match_nr) {
		char buf[PATH_MAX + 1];

		strncpy(buf, str, (len <= 0 ? PATH_MAX : (size_t) len));
		buf[(len <= 0 ? PATH_MAX : len) + 1]  = 0;

		for (size_t i = 0; i < exclude_match_nr; i++) {
			if (!regexec(&exclude_match[i], buf, 0, NULL, 0)) {
				if (verbose > 1)
					warnx("exclude path: %s", buf);
				return NULL;
			}
		}
	}

	new = xcalloc(1, sizeof(*new));

	new->src = (len < 0)
	           ? xstrdup(str)
	           : xstrndup(str, (size_t) len);

	if (verbose > 1)
		warnx("add to list: %s", new->src);

	if (inqueue) {
		if (inqueue->prev)
			errx(EX_SOFTWARE, "bad queue head");
		inqueue->prev = new;
		new->next = inqueue;
	}

	inqueue = new;
	queue_nr++;

	return new;
}

void dequeue_item(struct file *ptr)
{
	if (!ptr)
		return;
	if (ptr->prev)
		ptr->prev->next = ptr->next;
	if (ptr->next)
		ptr->next->prev = ptr->prev;
	if (inqueue == ptr)
		inqueue = NULL;
	queue_nr--;
}

void free_file(void *ptr)
{
	struct file *p = ptr;
	free(p->src);
	free(p->symlink);
	free(p);
}
