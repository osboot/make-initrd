// SPDX-License-Identifier: GPL-3.0-or-later

#include "config.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/utsname.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <err.h>
#include <errno.h>
#include <pthread.h>

#include <fts.h>

#include <libkmod.h>

#include "initrd-scanmod.h"

extern struct ruleset **filter_rules;

typedef const char *(get_list_key_t)(const struct kmod_list *);
typedef const char *(get_list_val_t)(const struct kmod_list *);

static int
match_filename(const char *filename, struct rule_pair **filters)
{
	int i               = 0;
	struct rule_pair *r = NULL;

	if (!filters)
		return 0;

	r = filters[i++];

	while (r) {
		int is_match = !regexec(r->value, filename, 0, NULL, 0);

		switch (r->type) {
			case RULE_MATCH:
				if (!is_match)
					return 0;
				break;
			case RULE_NOT_MATCH:
				if (is_match)
					return 0;
				break;
		}

		r = filters[i++];
	}

	return 1;
}

static int
match(struct kmod_list *symlist, struct rule_pair **filters, get_list_key_t *get_key, get_list_val_t *get_val)
{
	int i               = 0;
	struct rule_pair *r = NULL;
	struct kmod_list *l = NULL;

	if (!filters || !get_val)
		return 0;

	r = filters[i++];

	while (r) {
		int is_match = 0;

		kmod_list_foreach(l, symlist) {
			if (get_key) {
				const char *key = get_key(l);

				if (!key || strcmp(keywords[r->keyword], key))
					continue;
			}

			const char *val = get_val(l);

			if (!val)
				continue;

			if (!regexec(r->value, val, 0, NULL, 0)) {
				is_match = 1;
				break;
			}
		}

		switch (r->type) {
			case RULE_MATCH:
				if (!is_match)
					return 0;
				break;
			case RULE_NOT_MATCH:
				if (is_match)
					return 0;
				break;
		}

		r = filters[i++];
	}

	return 1;
}

static int
has_symbol_match(struct kmod_list *list, regex_t *preg, get_list_val_t *get_val)
{
	struct kmod_list *l = NULL;

	if (!list || !preg || !get_val)
		return 0;

	kmod_list_foreach(l, list) {
		const char *val = get_val(l);

		if (!val)
			continue;

		if (!regexec(preg, val, 0, NULL, 0))
			return 1;
	}

	return 0;
}

static int
match_symbols(struct kmod_list *provided, struct kmod_list *required, struct rule_pair **filters)
{
	int i               = 0;
	struct rule_pair *r = NULL;

	if (!filters)
		return 0;

	r = filters[i++];

	while (r) {
		int is_match = has_symbol_match(provided, r->value, &kmod_module_symbol_get_symbol) ||
		               has_symbol_match(required, r->value, &kmod_module_dependency_symbol_get_symbol);

		switch (r->type) {
			case RULE_MATCH:
				if (!is_match)
					return 0;
				break;
			case RULE_NOT_MATCH:
				if (is_match)
					return 0;
				break;
		}

		r = filters[i++];
	}

	return 1;
}

static pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;

struct module_data {
	struct kmod_ctx *ctx;
	const char *filename;
	struct kmod_module *mod;
	const char *modname;
	struct kmod_list *infolist;
	int info_state;
	struct kmod_list *provided;
	struct kmod_list *required;
	int symbols_state;
};

static int
init_module_mod(struct module_data *mdata)
{
	if (mdata->mod)
		return 0;

	if (kmod_module_new_from_path(mdata->ctx, mdata->filename, &mdata->mod) < 0)
		return -1;

	if (!(mdata->modname = kmod_module_get_name(mdata->mod)))
		return -1;

	return 0;
}

static int
process_module_ruleset(struct module_data *mdata, struct ruleset *set)
{
	int rc, is_match = 0;

	if (!set->flags)
		return 0;

	if (verbose > 1)
		warnx("%s: checking module against the ruleset patterns from %s ...", mdata->filename, set->filename);

	if (set->flags & RULESET_HAS_PATHS) {
		rc = match_filename(mdata->filename, set->paths);

		if (rc < 0)
			return 0;
		if (rc > 0) {
			is_match |= RULESET_HAS_PATHS;

			if (verbose > 1)
				warnx("%s: path matches", mdata->filename);
		} else {
			if (verbose > 1)
				warnx("%s: path does not match", mdata->filename);
			return 0;
		}
	}

	if (set->flags == is_match) {
		printf("%s\n", mdata->filename);
		return 1;
	}

	if (set->flags & RULESET_HAS_INFO) {
		if (init_module_mod(mdata) < 0)
			return 0;

		if (mdata->info_state == 0) {
			rc = kmod_module_get_info(mdata->mod, &mdata->infolist);
			if (rc >= 0) {
				mdata->info_state = 1;
			} else {
				errno = -rc;
				warn("Could not get information from '%s'", mdata->modname);
				mdata->info_state = -1;
				return 0;
			}
		} else if (mdata->info_state < 0) {
			return 0;
		}

		rc = match(mdata->infolist, set->info, &kmod_module_info_get_key, &kmod_module_info_get_value);
		if (rc < 0)
			return 0;
		if (rc > 0) {
			is_match |= RULESET_HAS_INFO;

			if (verbose > 1)
				warnx("%s: the module information matches", mdata->filename);
		} else {
			if (verbose > 1)
				warnx("%s: the module information does not match", mdata->filename);
			return 0;
		}
	}

	if (set->flags & RULESET_HAS_SYMBOLS) {
		if (init_module_mod(mdata) < 0)
			return 0;

		if (mdata->symbols_state == 0) {
			rc = kmod_module_get_symbols(mdata->mod, &mdata->provided);
			if (rc < 0 && rc != -ENOENT && rc != -ENODATA) {
				errno = -rc;
				warn("Could not get symbols from '%s'", mdata->modname);
				mdata->symbols_state = -1;
				return 0;
			}

			rc = kmod_module_get_dependency_symbols(mdata->mod, &mdata->required);
			if (rc < 0 && rc != -ENOENT && rc != -ENODATA) {
				errno = -rc;
				warn("Could not get dependency symbols from '%s'", mdata->modname);
				mdata->symbols_state = -1;
				return 0;
			}

			mdata->symbols_state = 1;
		} else if (mdata->symbols_state < 0) {
			return 0;
		}

		rc = match_symbols(mdata->provided, mdata->required, set->symbols);
		if (rc < 0)
			return 0;
		if (rc > 0) {
			is_match |= RULESET_HAS_SYMBOLS;

			if (verbose > 1)
				warnx("%s: symbols match", mdata->filename);
		} else {
			if (verbose > 1)
				warnx("%s: symbols do not match", mdata->filename);
			return 0;
		}
	}

	if (set->flags && set->flags == is_match) {
		pthread_mutex_lock(&print_mutex);
		printf("%s\n", mdata->filename);
		pthread_mutex_unlock(&print_mutex);
		return 1;
	}

	return 0;
}

static void
process_module(struct kmod_ctx *ctx, const char *filename)
{
	int i = 0;
	struct ruleset *set = NULL;
	struct module_data mdata = {
		.ctx = ctx,
		.filename = filename,
	};

	if (!filter_rules)
		return;

	set = filter_rules[i++];
	while (set) {
		if (process_module_ruleset(&mdata, set))
			break;
		set = filter_rules[i++];
	}

	if (mdata.infolist)
		kmod_module_info_free_list(mdata.infolist);
	if (mdata.provided)
		kmod_module_symbols_free_list(mdata.provided);
	if (mdata.required)
		kmod_module_dependency_symbols_free_list(mdata.required);
	if (mdata.mod)
		kmod_module_unref(mdata.mod);
}

static int
is_kernel_modname(const char *filename)
{
	const char *exts[] = {".ko", ".ko.zst", ".ko.xz", ".ko.gz", ".ko.bz2"};
	size_t num_exts = sizeof(exts) / sizeof(exts[0]);
	size_t len = strlen(filename);

	for (size_t i = 0; i < num_exts; i++) {
		size_t ext_len = strlen(exts[i]);
		if (len >= ext_len && !strcmp(filename + len - ext_len, exts[i]))
			return 1;
	}
	return 0;
}

struct module_queue {
	char **paths;
	size_t count;
	size_t capacity;
	size_t current;
	pthread_mutex_t mutex;
};

static const char *
queue_pop(struct module_queue *q)
{
	const char *path = NULL;

	pthread_mutex_lock(&q->mutex);
	if (q->current < q->count)
		path = q->paths[q->current++];
	pthread_mutex_unlock(&q->mutex);

	return path;
}

struct worker_data {
	const char *kerneldir;
	struct module_queue *queue;
};

static void *
worker_func(void *arg)
{
	struct worker_data *wdata = arg;
	struct kmod_ctx *ctx = kmod_new(wdata->kerneldir, NULL);
	const char *path;

	if (!ctx)
		err(EXIT_FAILURE, "%s: kmod_new", wdata->kerneldir);

	while ((path = queue_pop(wdata->queue)))
		process_module(ctx, path);

	kmod_unref(ctx);
	return NULL;
}

void
find_modules(const char *kerneldir)
{
	FTS *t;
	FTSENT *p;
	char *argv[2];
	struct module_queue q = { 0 };

	pthread_mutex_init(&q.mutex, NULL);

	argv[0] = (char *) kerneldir;
	argv[1] = NULL;

	if (!(t = fts_open(argv, FTS_PHYSICAL, NULL)))
		err(EXIT_FAILURE, "%s: fts_open", kerneldir);

	errno = 0;
	while ((p = fts_read(t))) {
		switch (p->fts_info) {
			case FTS_F:
			case FTS_SL:
				break;
			case FTS_DNR:
			case FTS_ERR:
			case FTS_NS:
				err(EXIT_FAILURE, "%s: fts_read", p->fts_path);
				break;
			case FTS_D:
			case FTS_DC:
			case FTS_SLNONE:
			// Ignore directories and broken symlinks
			default:
				continue;
		}

		if (!is_kernel_modname(p->fts_accpath))
			continue;

		if (q.count >= q.capacity) {
			q.capacity = q.capacity ? q.capacity * 2 : 256;
			q.paths    = xrealloc(q.paths, q.capacity, sizeof(char *));
		}
		q.paths[q.count++] = xstrdup(p->fts_path);
		errno = 0;
	}

	if (errno != 0)
		err(EXIT_FAILURE, "%s: fts_read", kerneldir);

	if (fts_close(t) < 0)
		err(EXIT_FAILURE, "%s: fts_close", kerneldir);

	if (!q.count) {
		pthread_mutex_destroy(&q.mutex);
		return;
	}

	int n_workers = jobs;
	if (n_workers <= 0) {
		long nprocs = sysconf(_SC_NPROCESSORS_ONLN);
		n_workers = (nprocs > 0) ? (int) nprocs : 1;
	}
	if ((size_t) n_workers > q.count)
		n_workers = (int) q.count;

	if (n_workers <= 1) {
		struct worker_data wdata = {
			.kerneldir = kerneldir,
			.queue     = &q,
		};
		worker_func(&wdata);
	} else {
		pthread_t *threads = xmalloc(sizeof(pthread_t) * (size_t) n_workers);
		struct worker_data *wdata = xmalloc(sizeof(struct worker_data) * (size_t) n_workers);
		int n_created = 0;

		for (int i = 0; i < n_workers; i++) {
			wdata[i].kerneldir = kerneldir;
			wdata[i].queue     = &q;

			if (pthread_create(&threads[i], NULL, worker_func, &wdata[i]) != 0) {
				warn("pthread_create");
				break;
			}
			n_created++;
		}

		for (int i = 0; i < n_created; i++)
			pthread_join(threads[i], NULL);

		free(threads);
		free(wdata);
	}

	for (size_t i = 0; i < q.count; i++)
		xfree(q.paths[i]);
	xfree(q.paths);
	pthread_mutex_destroy(&q.mutex);
}
