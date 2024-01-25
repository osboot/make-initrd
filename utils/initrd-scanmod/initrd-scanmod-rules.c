// SPDX-License-Identifier: GPL-3.0-or-later
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <regex.h>
#include <err.h>

#include "initrd-scanmod.h"

struct ruleset **filter_rules = NULL;

static keyword_t
get_keyword(char *value)
{
	int i = 0;
	while (keywords[i]) {
		if (!strcmp(keywords[i], value))
			return i;
		i++;
	}
	return KW_UNKNOWN_KEYWORD;
}

static void
parse_ruleset(const char *rulesfile, struct ruleset *set)
{
	size_t i;
	int n;
	char *nline, *a;
	struct rule_pair *rule = NULL;
	struct mapfile rules   = { 0 };

	size_t n_infos   = 0;
	size_t n_symbols = 0;
	size_t n_paths   = 0;

	if (open_map(rulesfile, &rules, 0) < 0)
		exit(EXIT_FAILURE);

	i = 1;
	a = rules.map;
	while (a && a[0]) {
		char *kw    = NULL;
		char *value = NULL;

		nline = strchr(a, '\n');

		while (isblank(a[0]))
			a++;

		if (a[0] == '#' || a[0] == '\n') {
			a = (nline) ? nline + 1 : NULL;
			i++;
			continue;
		}

		if (!nline)
			nline = a + strlen(a);

		rule       = xcalloc(1, sizeof(struct rule_pair));
		rule->type = RULE_NOT_MATCH;

		n = sscanf(a, "not-%ms %ms\n", &kw, &value);
		if (n != 2) {
			kw    = xfree(kw);
			value = xfree(value);

			rule->type = RULE_MATCH;
			n          = sscanf(a, "%ms %m[^\n]\n", &kw, &value);

			if (n != 2)
				errx(EXIT_FAILURE, "%s:%lu: bad line format", rulesfile, i);
		}

		if ((rule->keyword = get_keyword(kw)) == KW_UNKNOWN_KEYWORD)
			errx(EXIT_FAILURE, "%s:%lu: unknown keyword", rulesfile, i);
		xfree(kw);

		rule->value = xmalloc(sizeof(regex_t));

		if (regcomp(rule->value, value, REG_NOSUB | REG_EXTENDED))
			errx(EXIT_FAILURE, "%s:%lu: '%s' is not a regular expression", rulesfile, i, value);
		xfree(value);

		switch (rule->keyword) {
			case KW_SYMBOL:
				set->symbols            = xrealloc(set->symbols, (n_symbols + 1), sizeof(void *));
				set->symbols[n_symbols] = rule;
				n_symbols++;

				set->flags |= RULESET_HAS_SYMBOLS;
				break;
			case KW_FILENAME:
				set->paths          = xrealloc(set->paths, (n_paths + 1), sizeof(void *));
				set->paths[n_paths] = rule;
				n_paths++;

				set->flags |= RULESET_HAS_PATHS;
				break;
			case KW_ALIAS:
			case KW_AUTHOR:
			case KW_DEPENDS:
			case KW_DESCRIPTION:
			case KW_FIRMWARE:
			case KW_LICENSE:
			case KW_NAME:
				set->info          = xrealloc(set->info, (n_infos + 1), sizeof(void *));
				set->info[n_infos] = rule;
				n_infos++;

				set->flags |= RULESET_HAS_INFO;
				break;
			case KW_UNKNOWN_KEYWORD:
				break;
		}

		a = nline + 1;
		i++;
	}

	set->info          = xrealloc(set->info, (n_infos + 1), sizeof(void *));
	set->info[n_infos] = NULL;

	set->symbols            = xrealloc(set->symbols, (n_symbols + 1), sizeof(void *));
	set->symbols[n_symbols] = NULL;

	set->paths          = xrealloc(set->paths, (n_paths + 1), sizeof(void *));
	set->paths[n_paths] = NULL;

	close_map(&rules);
}

static int
filename_in_ruleset(char *filename)
{
	int i               = 0;
	struct ruleset *set = NULL;

	if (!filter_rules)
		return 0;

	set = filter_rules[i++];
	while (set) {
		if (!strcmp(set->filename, filename))
			return 1;
		set = filter_rules[i++];
	}
	return 0;
}

void
parse_rules(int n_files, char **files)
{
	int i;
	size_t n_rules = 0;

	for (i = 0; i < n_files; i++) {
		if (!files || !files[i] || filename_in_ruleset(files[i]))
			continue;

		struct ruleset *set = xcalloc(1, sizeof(struct ruleset));
		set->filename       = files[i];

		parse_ruleset(files[i], set);

		filter_rules          = xrealloc(filter_rules, (n_rules + 1), sizeof(void *));
		filter_rules[n_rules] = set;

		n_rules++;
	}

	filter_rules          = xrealloc(filter_rules, (n_rules + 1), sizeof(void *));
	filter_rules[n_rules] = NULL;
}

static void
free_ruleset(struct rule_pair **filters)
{
	int i               = 0;
	struct rule_pair *r = NULL;

	if (!filters)
		return;

	r = filters[i++];
	while (r) {
		regfree(r->value);
		xfree(r->value);
		xfree(r);
		r = filters[i++];
	}
	xfree(filters);
}

void
free_rules(void)
{
	int i               = 0;
	struct ruleset *set = NULL;

	if (!filter_rules)
		return;

	set = filter_rules[i++];
	while (set) {
		free_ruleset(set->info);
		free_ruleset(set->paths);
		free_ruleset(set->symbols);
		xfree(set);
		set = filter_rules[i++];
	}

	xfree(filter_rules);
}
