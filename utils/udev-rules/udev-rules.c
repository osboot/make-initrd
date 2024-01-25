// SPDX-License-Identifier: GPL-3.0-or-later
#include <sys/stat.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <libgen.h>
#include <err.h>

#include "udev-rules.h"

static void __attribute__((noreturn))
print_help(const char *progname, int rc)
{
	printf("Usage: %s [options] [file|directory]...\n"
	       "\n"
	       "The utility parses udev rules and looks for errors in them.\n"
	       "It can also extract some information from the rules.\n"
	       "\n"
	       "All of warnings have both positive and negative forms. The negative\n"
	       "form of -Wfoo is -Wno-foo.\n"
	       "\n"
	       "Options:\n"
	       "  -Wall                   This enables all the warnings;\n"
	       "  -Werror                 Treat all warnings as errors;\n"
	       "  -Wmissing-label         Warn about GOTO without corresponding LABEL;\n"
	       "  -Wunsed-labels          Warn about unused labels;\n"
	       "  -Wdup-match             Warn about duplicate matches in a rule;\n"
	       "  -Wconflict-match        Warn about conflicting matches in a rule\n"
	       "                          (entry is checked for == and != at the same time);\n"
	       "  -Wincomplete-rules      Warn about rule contains only match conditions without any action;\n"
	       "  -Wmulti-label           Warn about multiple LABEL in same rule;\n"
	       "  -Wmulti-goto            Warn about multiple GOTO in same rule;\n"
	       "  -x                      Show external file and commands that are used in the rules;\n"
	       "  -h                      Show this text and exit.\n"
	       "\n",
	       progname);
	exit(rc);
}

const char *warning_str[_W_TYPE_MAX] = {
	[W_ERROR]             = "error",
	[W_MISSING_LABEL]     = "missing-label",
	[W_UNUSED_LABELS]     = "unsed-labels",
	[W_DUP_MATCH]         = "dup-match",
	[W_CONFLICT_MATCH]    = "conflict-match",
	[W_INCOMPLETE_RULES]  = "incomplete-rule",
	[W_MULTI_LABEL]       = "multi-label",
	[W_MULTI_GOTO]        = "multi-goto",
};

int main(int argc, char **argv)
{
	int c, i, set;
	struct rules_state state = { 0 };

	LIST_HEAD(files);
	LIST_HEAD(rules);
	LIST_HEAD(gotos);
	LIST_HEAD(labels);

	state.rules = &rules;
	state.files = &files;
	state.gotos = &gotos;
	state.labels = &labels;

	while ((c = getopt(argc, argv, "hW:x")) != -1) {
		switch (c) {
			case 'W':
				set = !!strncmp(optarg, "no-", 3);

				i = 0;
				if (!strcmp(optarg + (set ? 0 : 3), "all")) {
					memset(state.warning, set, sizeof(state.warning));
					state.warning[W_ERROR] = 0;
				} else for (; i < _W_TYPE_MAX; i++) {
						if (!strcmp(optarg + (set ? 0 : 3), warning_str[i])) {
							state.warning[i] = set;
							break;
						}
					}
				if (i == _W_TYPE_MAX)
					errx(EXIT_FAILURE, "unknown option: -W%s", optarg);
				break;
			case 'x':
				state.show_external = 1;
				break;
			case 'h':
				print_help(basename(argv[0]), EXIT_SUCCESS);
				break;
			default:
				print_help(basename(argv[0]), EXIT_FAILURE);
				break;
		}
	}

	if (optind >= argc)
		errx(EXIT_FAILURE, "Error: missing directory name");

	while (optind < argc) {
		char *filename;
		struct stat st;

		if (stat(argv[optind], &st) < 0)
			err(EXIT_FAILURE, "Error: %s", argv[optind]);

		switch (st.st_mode & S_IFMT) {
			case S_IFDIR:
				if (rules_readdir(argv[optind], &state) < 0) {
					state.retcode = 1;
					goto end;
				}
				break;
			case S_IFREG:
				filename = strdup(argv[optind]);
				if (!filename)
					err(EXIT_FAILURE, "strdup");

				if (rules_readfile(filename, &state) < 0) {
					state.retcode = 1;
					goto end;
				}
				break;
			default:
				warnx("%s: the type of the argument is neither a file nor a directory. crazy?",
				      argv[optind]);
				state.retcode = 1;
				goto end;
		}
		optind++;
	}

	check_goto_label(&state);

	int warn_header = 0;

	for (i = 0; i < _W_TYPE_MAX; i++) {
		if (!state.warning_summary[i])
			continue;
		if (!warn_header) {
			warnx("For more information:");
			warn_header = 1;
		}
		warnx("  https://github.com/osboot/make-initrd/wiki/udev-rules-W%s", warning_str[i]);
	}
end:
	free_goto_label(&gotos);
	free_goto_label(&labels);

	while (!list_empty(&rules)) {
		struct rule *rule = list_first_entry(&rules, struct rule, list);
		list_del(&rule->list);

		while (!list_empty(&rule->pairs)) {
			struct rule_pair *pair = list_first_entry(&rule->pairs, struct rule_pair, list);
			list_del(&pair->list);

			free_string(pair->attr);
			free_string(pair->value);
			free(pair);
		}
		free(rule);
	}

	while (!list_empty(&files)) {
		struct rule_file *file = list_first_entry(&files, struct rule_file, list);
		list_del(&file->list);
		free(file->name);
		free(file);
	}

	return state.retcode;
}
