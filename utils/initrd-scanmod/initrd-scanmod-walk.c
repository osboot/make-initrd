#define _GNU_SOURCE
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/utsname.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <err.h>
#include <errno.h>

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
	int rc              = 0;
	struct rule_pair *r = NULL;

	if (!filters)
		return rc;

	r = filters[i++];

	while (r) {
		if (!regexec(r->value, filename, 0, NULL, 0)) {
			switch (r->type) {
				case RULE_NOT_MATCH:
					return -1;
				case RULE_MATCH:
					rc = 1;
			}
		}
		r = filters[i++];
	}

	return rc;
}

static int
match(struct kmod_list *symlist, struct rule_pair **filters, get_list_key_t *get_key, get_list_val_t *get_val)
{
	int i               = 0;
	int rc              = 0;
	struct rule_pair *r = NULL;
	struct kmod_list *l = NULL;

	if (!filters)
		return rc;

	r = filters[i++];

	while (r) {
		kmod_list_foreach(l, symlist) {
			if (get_key) {
				const char *key = get_key(l);

				if (!key || strcmp(keywords[r->keyword], key))
					continue;
			}

			if (!get_val)
				continue;

			const char *val = get_val(l);

			if (!val || regexec(r->value, val, 0, NULL, 0))
				continue;

			switch (r->type) {
				case RULE_NOT_MATCH:
					return -1;
				case RULE_MATCH:
					rc = 1;
			}
		}
		r = filters[i++];
	}

	return rc;
}

static int
process_module_ruleset(struct kmod_ctx *ctx, const char *filename, struct ruleset *set)
{
	int rc, is_match = 0;
	const char *modname;
	struct kmod_module *mod = NULL;
	struct kmod_list *infolist, *symlist;

	symlist = infolist = NULL;

	if (kmod_module_new_from_path(ctx, filename, &mod) < 0)
		goto exit;

	if (!(modname = kmod_module_get_name(mod)))
		goto exit;

	if (verbose > 1)
		warnx("%s: checking module against the ruleset patterns from %s ...", kmod_module_get_path(mod), set->filename);

	if (set->flags & RULESET_HAS_PATHS) {
		rc = match_filename(kmod_module_get_path(mod), set->paths);

		if (rc < 0)
			goto exit;
		if (rc > 0) {
			is_match |= RULESET_HAS_PATHS;

			if (verbose > 1)
				warnx("%s: path matches", kmod_module_get_path(mod));
		} else {
			if (verbose > 1)
				warnx("%s: path does not match", kmod_module_get_path(mod));
		}
	}

	if (set->flags & RULESET_HAS_INFO) {
		rc = kmod_module_get_info(mod, &infolist);

		if (rc > 0) {
			rc = match(infolist, set->info, &kmod_module_info_get_key, &kmod_module_info_get_value);

			kmod_module_info_free_list(infolist);
			infolist = NULL;

			if (rc < 0)
				goto exit;
			if (rc > 0) {
				is_match |= RULESET_HAS_INFO;

				if (verbose > 1)
					warnx("%s: the module information matches", kmod_module_get_path(mod));
			} else {
				if (verbose > 1)
					warnx("%s: the module information does not match", kmod_module_get_path(mod));
			}

		} else if (rc < 0) {
			errno = -rc;
			warn("Could not get information from '%s'", modname);
			goto exit;
		}
	}

	if (set->flags & RULESET_HAS_SYMBOLS) {
		rc = kmod_module_get_symbols(mod, &symlist);

		if (rc > 0) {
			rc = match(symlist, set->symbols, NULL, &kmod_module_symbol_get_symbol);

			kmod_module_symbols_free_list(symlist);
			symlist = NULL;

			if (rc < 0)
				goto exit;
			if (rc > 0) {
				is_match |= RULESET_HAS_SYMBOLS;

				if (verbose > 1)
					warnx("%s: symbols matches", kmod_module_get_path(mod));
			} else {
				if (verbose > 1)
					warnx("%s: symbols does not match", kmod_module_get_path(mod));
			}

		} else if (rc < 0 && rc != -ENOENT) {
			errno = -rc;
			warn("Could not get symbols from '%s'", modname);
			goto exit;
		}

		rc = kmod_module_get_dependency_symbols(mod, &symlist);

		if (rc > 0) {
			rc = match(symlist, set->symbols, NULL, &kmod_module_dependency_symbol_get_symbol);

			kmod_module_dependency_symbols_free_list(symlist);
			symlist = NULL;

			if (rc < 0)
				goto exit;
			if (rc > 0) {
				is_match |= RULESET_HAS_SYMBOLS;

				if (verbose > 1)
					warnx("%s: symbols matches", kmod_module_get_path(mod));
			} else {
				if (verbose > 1)
					warnx("%s: symbols does not match", kmod_module_get_path(mod));
			}

		} else if (rc < 0 && rc != -ENOENT) {
			errno = -rc;
			warn("Could not get dependency symbols from '%s'", modname);
			goto exit;
		}
	}
	if (set->flags == is_match)
		printf("%s\n", kmod_module_get_path(mod));
exit:
	if (mod)
		kmod_module_unref(mod);

	return is_match;
}

static void
process_module(struct kmod_ctx *ctx, const char *filename)
{
	int i               = 0;
	struct ruleset *set = NULL;

	if (!filter_rules)
		return;

	set = filter_rules[i++];
	while (set) {
		if (process_module_ruleset(ctx, filename, set))
			return;
		set = filter_rules[i++];
	}
}

static int
dsort(const FTSENT **a, const FTSENT **b)
{
	if (S_ISDIR((*a)->fts_statp->st_mode)) {
		if (!S_ISDIR((*b)->fts_statp->st_mode))
			return 1;
	} else if (S_ISDIR((*b)->fts_statp->st_mode))
		return -1;
	return (strcmp((*a)->fts_name, (*b)->fts_name));
}

static int
is_kernel_modname(const char *filename)
{
	size_t len = strlen(filename);
	if (len < 3)
		return 0;

	if (!strcmp(".ko", filename + len - 3))
		return 1;

	if (len <= 6)
		return 0;

	if (!strcmp(".ko.gz", filename + len - 6))
		return 1;

	if (!strcmp(".ko.xz", filename + len - 6))
		return 1;

	return 0;
}

void
find_modules(const char *kerneldir)
{
	struct kmod_ctx *ctx = NULL;
	FTS *t;
	FTSENT *p;
	char *argv[2];

	if (!(ctx = kmod_new(kerneldir, NULL)))
		err(EXIT_FAILURE, "%s: kmod_new", kerneldir);

	argv[0] = (char *) kerneldir;
	argv[1] = NULL;

	if (!(t = fts_open(argv, FTS_PHYSICAL, dsort)))
		err(EXIT_FAILURE, "%s: fts_open", kerneldir);

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

		process_module(ctx, p->fts_path);
	}

	if (fts_close(t) < 0)
		err(EXIT_FAILURE, "%s: fts_close", kerneldir);

	kmod_unref(ctx);
}
