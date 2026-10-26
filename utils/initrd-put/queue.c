// SPDX-License-Identifier: GPL-3.0-or-later

#include "config.h"

#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sysexits.h>
#include <err.h>
#include <regex.h>
#include <search.h>

#include "memory.h"
#include "queue.h"
#include "tree.h"

extern int verbose;
extern char *prefix;
extern regex_t *exclude_match;
extern size_t exclude_match_nr;

static struct file *inqueue = NULL;
static size_t queue_nr = 0;
static void *queued_files = NULL;

static int compare_file(const void *a, const void *b)
{
	return strcmp(((const struct file *)a)->src, ((const struct file *)b)->src);
}

static struct file *queue_find(const char *path)
{
	struct file key = { .src = (char *) path };
	void *res = tfind(&key, &queued_files, compare_file);
	if (!res)
		return NULL;
	return *(struct file **) res;
}

bool is_path_queued(const char *path)
{
	return queue_find(path) != NULL;
}

struct file *get_queue(size_t *nr)
{
	if (nr)
		*nr = queue_nr;

	return inqueue;
}

struct file *enqueue_item(const char *str, ssize_t len)
{
	struct file *new;
	char buf[PATH_MAX + 1];
	const char *src_str = str;

	if (len >= 0) {
		size_t n = (size_t) len;

		if (n > PATH_MAX)
			n = PATH_MAX;

		memcpy(buf, str, n);
		buf[n] = '\0';
		src_str = buf;
	}

	if (exclude_match_nr) {
		for (size_t i = 0; i < exclude_match_nr; i++) {
			if (!regexec(&exclude_match[i], src_str, 0, NULL, 0)) {
				if (verbose > 1)
					warnx("exclude path: %s", src_str);
				return NULL;
			}
		}
	}

	new = queue_find(src_str);
	if (new)
		return new;

	if (is_path_added(src_str))
		return NULL;

	new = xcalloc(1, sizeof(*new));
	new->src = xstrdup(src_str);

	if (verbose > 1)
		warnx("add to list: %s", new->src);

	void *ptr = tsearch(new, &queued_files, compare_file);
	if (!ptr)
		err(EX_OSERR, "tsearch");

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

	tdelete(ptr, &queued_files, compare_file);

	if (ptr->prev)
		ptr->prev->next = ptr->next;
	if (ptr->next)
		ptr->next->prev = ptr->prev;
	if (inqueue == ptr)
		inqueue = ptr->next;
	queue_nr--;
}

void queue_destroy(void)
{
#ifdef HAVE_TDESTROY
	tdestroy(queued_files, free_file);
#endif
	queued_files = NULL;
	inqueue = NULL;
	queue_nr = 0;
}

void free_file(void *ptr)
{
	struct file *p = ptr;
	free(p->src);
	free(p->symlink);
	free(p);
}
