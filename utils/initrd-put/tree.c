// SPDX-License-Identifier: GPL-3.0-or-later

#include "config.h"

#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <search.h>
#include <sysexits.h>
#include <err.h>

#include "queue.h"
#include "tree.h"

static void *files = NULL;
static size_t tree_files_nr = 0;

static int compare(const void *a, const void *b) __attribute__((__nonnull__ (1, 2)));
static void walk_action(const void *nodep, VISIT which, void *closure) __attribute__((__nonnull__ (1, 3)));;

int compare(const void *a, const void *b)
{
	return strcmp(((struct file *)a)->src, ((struct file *)b)->src);
}

bool is_path_added(const char *path)
{
	struct file v = { 0 };
	v.src = (char *) path;
	return (tfind(&v, &files, compare) != NULL) || is_path_queued(path);
}

bool tree_add_file(struct file *file)
{
	void *ptr = tsearch(file, &files, compare);

	if (!ptr)
		err(EX_OSERR, "tsearch");

	bool added = ((*(struct file **)ptr) == file);
	if (added)
		tree_files_nr++;

	return added;
}

size_t tree_get_nr(void)
{
	return tree_files_nr;
}

void tree_destroy(void)
{
#ifdef HAVE_TDESTROY
	tdestroy(files, free_file);
#endif
	files = NULL;
	tree_files_nr = 0;
}

void walk_action(const void *nodep, VISIT which, void *closure)
{
	struct file *p;
	void (*file_handler)(struct file *p) = closure;

	switch (which) {
		case preorder:
			break;
		case postorder:
			p = *(struct file **) nodep;
			file_handler(p);
			break;
		case endorder:
			break;
		case leaf:
			p = *(struct file **) nodep;
			file_handler(p);
			break;
	}
}

#ifndef HAVE_TWALK_R
static void *twalk_r_closure;
static void (*twalk_r_action)(const void *nodep, VISIT which, void *closure);

static void twalk_handler(const void *nodep, VISIT which,
                          int depth __attribute__((unused)))
{
	twalk_r_action(nodep, which, twalk_r_closure);
}

static void twalk_r(const void *root,
                    void (*action)(const void *nodep, VISIT which, void *closure),
                    void *closure)
{
	twalk_r_action  = action;
	twalk_r_closure = closure;
	twalk(root, twalk_handler);
}
#endif

void tree_walk(void (*handler)(struct file *))
{
	twalk_r(files, walk_action, handler);
}
