/* SPDX-License-Identifier: GPL-2.0-or-later */

%{
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <err.h>

#include "udev-rules.h"
#include "udev-rules-parser.h"
#include "udev-rules-scanner.h"

static int yyerror(yyscan_t scanner, struct rules_state *state, const char *s)
{
	warnx("%s:%d: %s", state->cur_file->name, yyget_lineno(scanner), s);
	state->retcode = 1;
	return 1;
}

static struct rule *get_rule(yyscan_t scanner, struct rules_state *state)
{
	struct rule *new;

	if (state->cur_rule)
		return state->cur_rule;

	if (!(new = calloc(1, sizeof(*new))))
		err(1, "malloc(rule)");

	new->global_order = state->global_rule_nr;
	new->file = state->cur_file;
	new->line_nr = yyget_lineno(scanner);

	INIT_LIST_HEAD(&new->pairs);
	state->cur_rule = new;

	list_add_tail(&new->list, state->rules);
	return state->cur_rule;
}

static struct rule_pair *get_pair(struct rule *rule)
{
	struct rule_pair *new;

	if (!(new = calloc(1, sizeof(*new))))
		err(1, "malloc(rule_pair)");

	new->rule = rule;

	list_add_tail(&new->list, &rule->pairs);
	return new;
}

static const char *key2str(int n)
{
	switch (n) {
		case KEY_ACTION: return "ACTION";
		case KEY_ATTR: return "ATTR";
		case KEY_ATTRS: return "ATTRS";
		case KEY_CONST: return "CONST";
		case KEY_DEVPATH: return "DEVPATH";
		case KEY_DRIVER: return "DRIVER";
		case KEY_DRIVERS: return "DRIVERS";
		case KEY_ENV: return "ENV";
		case KEY_GOTO: return "GOTO";
		case KEY_GROUP: return "GROUP";
		case KEY_IMPORT: return "IMPORT";
		case KEY_KERNEL: return "KERNEL";
		case KEY_KERNELS: return "KERNELS";
		case KEY_LABEL: return "LABEL";
		case KEY_MODE: return "MODE";
		case KEY_NAME: return "NAME";
		case KEY_OPTIONS: return "OPTIONS";
		case KEY_OWNER: return "OWNER";
		case KEY_PROGRAM: return "PROGRAM";
		case KEY_RESULT: return "RESULT";
		case KEY_RUN: return "RUN";
		case KEY_SECLABEL: return "SECLABEL";
		case KEY_SUBSYSTEM: return "SUBSYSTEM";
		case KEY_SUBSYSTEMS: return "SUBSYSTEMS";
		case KEY_SYMLINK: return "SYMLINK";
		case KEY_SYSCTL: return "SYSCTL";
		case KEY_TAG: return "TAG";
		case KEY_TAGS: return "TAGS";
		case KEY_TEST: return "TEST";
	}
	return NULL;
}

static const char *op2str(int n)
{
	switch (n) {
		case OP_ASSIGN:       return "=";
		case OP_ASSIGN_FINAL: return ":=";
		case OP_ADD:          return "+=";
		case OP_REMOVE:       return "-=";
		case OP_MATCH:        return "==";
		case OP_NOMATCH:      return "!=";
	}
	return NULL;
}

static bool in_list(const char *k, ...)
{
	char *s = NULL;
	va_list ap;
	va_start(ap, k);
	while ((s = va_arg(ap, char *)) != NULL) {
		if (*s && !strcmp(k, s))
			break;
	}
	va_end(ap);
	return (s != NULL);
}

static void rule_log_dup_entry(yyscan_t scanner, struct rules_state *state, struct rule_pair *pair, int dups)
{
	warnx("%s:%d: %s%s%s%s%s\"%s\" is duplicated %d times in the same match block [-W%s]",
		state->cur_file->name,
		yyget_lineno(scanner),
		key2str(pair->key),
		( pair->attr ? "{" : "" ), ( pair->attr ? pair->attr->string : "" ), ( pair->attr ? "}" : "" ),
		op2str(pair->op),
		pair->value->string,
		dups,
		warning_str[W_DUP_MATCH]);
	warning_update_retcode(state);
}

static void rule_log_conflict_match(yyscan_t scanner, struct rules_state *state, struct rule_pair *pair)
{
	warnx("%s:%d: %s%s%s%s is checked for == and != at the same time [-W%s]",
		state->cur_file->name,
		yyget_lineno(scanner),
		key2str(pair->key),
		( pair->attr ? "{" : "" ), ( pair->attr ? pair->attr->string : "" ), ( pair->attr ? "}" : "" ),
		warning_str[W_CONFLICT_MATCH]);
	warning_update_retcode(state);
}

static void rule_log_invalid_attr(yyscan_t scanner, struct rules_state *state, rule_key_t key)
{
	warnx("%s:%d: invalid attribute for %s.",
		state->cur_file->name, yyget_lineno(scanner), key2str(key));
	state->retcode = 1;
}

static void rule_log_invalid_op(yyscan_t scanner, struct rules_state *state, rule_key_t key, rule_op_t op)
{
	warnx("%s:%d: %s can not have `%s' operator.",
		state->cur_file->name, yyget_lineno(scanner), key2str(key), op2str(op));
	state->retcode = 1;
}

static void check_match_conditions(yyscan_t scanner, struct rules_state *state, struct rule_pair *pair)
{
	struct rule_pair *p;
	struct rule *rule = pair->rule;
	bool is_match = (pair->op < _OP_TYPE_IS_MATCH);
	int dups_nr = 0;
	int conflict_nr = 0;

	// For now, ignore changes to variables.
	if (!is_match)
		return;

	list_for_each_entry(p, &rule->pairs, list) {
		if (pair == p)
			continue;

		if (p->op > _OP_TYPE_IS_MATCH) {
			if (dups_nr)
				rule_log_dup_entry(scanner, state, pair, dups_nr + 1);
			if (conflict_nr)
				rule_log_conflict_match(scanner, state, pair);
			dups_nr = 0;
			conflict_nr = 0;
		}

		if (pair->key != p->key ||
		    strcmp(pair->value->string, p->value->string))
			continue;

		if (pair->attr && p->attr) {
			if (strcmp(pair->attr->string, p->attr->string))
				continue;
		} else if (pair->attr || p->attr) {
			continue;
		}

		if (pair->op != p->op) {
			if (state->warning[W_CONFLICT_MATCH])
				conflict_nr++;
		} else {
			if (state->warning[W_DUP_MATCH])
				dups_nr++;
		}
	}

	if (dups_nr)
		rule_log_dup_entry(scanner, state, pair, dups_nr + 1);

	if (conflict_nr)
		rule_log_conflict_match(scanner, state, pair);
}


static void process_token(yyscan_t scanner, struct rules_state *state, struct rule_pair *pair)
{
	struct rule_goto_label *label;
	bool is_match = (pair->op < _OP_TYPE_IS_MATCH);

	switch (pair->key) {
		case KEY_ACTION:
			if (pair->attr)
				rule_log_invalid_attr(scanner, state, pair->key);

			if (!is_match)
				rule_log_invalid_op(scanner, state, pair->key, pair->op);
			break;
		case KEY_ATTR:
		case KEY_SYSCTL:
			if (!pair->attr || isempty(pair->attr->string))
				rule_log_invalid_attr(scanner, state, pair->key);

			if (pair->op == OP_REMOVE)
				rule_log_invalid_op(scanner, state, pair->key, pair->op);

			if (pair->op == OP_ADD || pair->op == OP_ASSIGN_FINAL) {
				warnx("%s:%d: %s key takes '==', '!=', or '=' operator.",
					state->cur_file->name, yyget_lineno(scanner), key2str(pair->key));
				warning_update_retcode(state);
				pair->op = OP_ASSIGN;
			}
			break;
		case KEY_ATTRS:
			if (!pair->attr || isempty(pair->attr->string))
				rule_log_invalid_attr(scanner, state, pair->key);

			if (!is_match)
				rule_log_invalid_op(scanner, state, pair->key, pair->op);
			break;
		case KEY_CONST:
			if (!pair->attr || isempty(pair->attr->string))
				rule_log_invalid_attr(scanner, state, pair->key);

			if (!in_list(pair->attr->string, "arch", "virt", NULL)) {
				warnx("%s:%d: unknown attribute \"%s\". This rule will never match.",
					state->cur_file->name,
					yyget_lineno(scanner),
					pair->attr->string);
				warning_update_retcode(state);
			}

			if (!is_match)
				rule_log_invalid_op(scanner, state, pair->key, pair->op);
			break;
		case KEY_DEVPATH:
		case KEY_DRIVER:
		case KEY_DRIVERS:
		case KEY_KERNEL:
		case KEY_KERNELS:
		case KEY_SUBSYSTEMS:
			if (pair->attr)
				rule_log_invalid_attr(scanner, state, pair->key);

			if (!is_match)
				rule_log_invalid_op(scanner, state, pair->key, pair->op);
			break;
		case KEY_SUBSYSTEM:
			if (pair->attr)
				rule_log_invalid_attr(scanner, state, pair->key);

			if (!is_match)
				rule_log_invalid_op(scanner, state, pair->key, pair->op);

			if (in_list(pair->value->string, "bus", "class", NULL)) {
				warnx("%s:%d: \"%s\" must be specified as \"subsystem\"..",
					state->cur_file->name,
					yyget_lineno(scanner),
					pair->value->string);
				warning_update_retcode(state);
			}
			break;
		case KEY_ENV:
			if (!pair->attr || isempty(pair->attr->string))
				rule_log_invalid_attr(scanner, state, pair->key);

			if (pair->op == OP_REMOVE)
				rule_log_invalid_op(scanner, state, pair->key, pair->op);

			if (pair->op == OP_ASSIGN_FINAL) {
				warnx("%s:%d: %s key takes '==', '!=', '=', or '+=' operator.",
					state->cur_file->name, yyget_lineno(scanner), key2str(pair->key));
				warning_update_retcode(state);
				pair->op = OP_ASSIGN;
			}

			if (!is_match) {
				if (in_list(pair->attr->string,
				            "ACTION", "DEVLINKS", "DEVNAME", "DEVPATH", "DEVTYPE", "DRIVER",
				            "IFINDEX", "MAJOR", "MINOR", "SEQNUM", "SUBSYSTEM", "TAGS", NULL)) {
					warnx("%s:%d: invalid ENV attribute. '%s' cannot be set.",
						state->cur_file->name,
						yyget_lineno(scanner),
						pair->attr->string);
					warning_update_retcode(state);
				}
		}
			break;
		case KEY_GOTO:
			if (pair->attr)
				rule_log_invalid_attr(scanner, state, pair->key);

			if (pair->op != OP_ASSIGN)
				rule_log_invalid_op(scanner, state, pair->key, pair->op);

			if (pair->rule->has_goto) {
				warnx("%s:%d: Contains multiple GOTO keys.",
					state->cur_file->name,
					yyget_lineno(scanner));
				warning_update_retcode(state);
			}

			pair->rule->has_goto = 1;
			label = get_goto(state);
			label->name = pair->value;
			label->rule = state->cur_rule;

			break;
		case KEY_LABEL:
			if (pair->attr)
				rule_log_invalid_attr(scanner, state, pair->key);
			if (pair->op != OP_ASSIGN)
				rule_log_invalid_op(scanner, state, pair->key, pair->op);

			label = get_label(state);
			label->name = pair->value;
			label->rule = state->cur_rule;

			break;
		case KEY_OWNER:
		case KEY_GROUP:
		case KEY_MODE:
		case KEY_OPTIONS:
			if (pair->attr)
				rule_log_invalid_attr(scanner, state, pair->key);

			if (is_match || pair->op == OP_REMOVE)
				rule_log_invalid_op(scanner, state, pair->key, pair->op);

			break;
		case KEY_IMPORT:
			if (!pair->attr || isempty(pair->attr->string))
				rule_log_invalid_attr(scanner, state, pair->key);

			if (pair->op == OP_REMOVE)
				rule_log_invalid_op(scanner, state, pair->key, pair->op);

			if (!in_list(pair->attr->string,
			             "file", "program", "builtin",
			             "db", "cmdline", "parent", NULL)) {
				warnx("%s:%d: unknown attribute \"%s\".",
					state->cur_file->name,
					yyget_lineno(scanner),
					pair->attr->string);
				warning_update_retcode(state);
			}

			if (state->show_external &&
			    in_list(pair->attr->string, "file", "program", "builtin", NULL)) {
				fprintf(stdout, "%s:%d\timport\t%s\t%s\n",
					state->cur_file->name,
					yyget_lineno(scanner),
					pair->attr->string,
					pair->value->string);
			}
			break;
		case KEY_NAME:
			if (pair->attr)
				rule_log_invalid_attr(scanner, state, pair->key);

			if (pair->op == OP_REMOVE)
				rule_log_invalid_op(scanner, state, pair->key, pair->op);

			if (pair->op == OP_ADD) {
				warnx("%s:%d: %s key takes '==', '!=', '=', or ':=' operator.",
					state->cur_file->name, yyget_lineno(scanner), key2str(pair->key));
				warning_update_retcode(state);
				pair->op = OP_ASSIGN;
			}

			if (!is_match) {
				if (!strcmp(pair->value->string, "%k")) {
					warnx("%s:%d: ignoring NAME=\"%%k\", as it will take no effect.",
						state->cur_file->name, yyget_lineno(scanner));
					warning_update_retcode(state);
				}
				if (isempty(pair->value->string)) {
					warnx("%s:%d: ignoring NAME=\"\", as udev will not delete any network interfaces.",
						state->cur_file->name,
						yyget_lineno(scanner));
					warning_update_retcode(state);
				}
			}
			break;
		case KEY_PROGRAM:
			if (pair->attr)
				rule_log_invalid_attr(scanner, state, pair->key);

			if (pair->op == OP_REMOVE)
				rule_log_invalid_op(scanner, state, pair->key, pair->op);

			if (state->show_external)
				fprintf(stdout, "%s:%d\tprogram\tprogram\t%s\n",
					state->cur_file->name,
					yyget_lineno(scanner),
					pair->value->string);
			break;
		case KEY_RESULT:
		case KEY_SYMLINK:
			if (pair->attr)
				rule_log_invalid_attr(scanner, state, pair->key);
			break;
		case KEY_RUN:
			if (!pair->attr) {
				free_string(pair->attr);
				pair->attr = new_string();
				string_append_str(pair->attr, "program", 7);
			}

			if (is_match || pair->op == OP_REMOVE)
				rule_log_invalid_op(scanner, state, pair->key, pair->op);

			if (!in_list(pair->attr->string, "builtin", "program", NULL)) {
				warnx("%s:%d: unknown attribute \"%s\".",
					state->cur_file->name,
					yyget_lineno(scanner),
					pair->attr->string);
				warning_update_retcode(state);
			}

			if (state->show_external)
				fprintf(stdout, "%s:%d\trun\t%s\t%s\n",
					state->cur_file->name,
					yyget_lineno(scanner),
					pair->attr->string,
					pair->value->string);
			break;
		case KEY_SECLABEL:
			if (!pair->attr || isempty(pair->attr->string))
				rule_log_invalid_attr(scanner, state, pair->key);

			if (is_match || pair->op == OP_REMOVE)
				rule_log_invalid_op(scanner, state, pair->key, pair->op);

			if (pair->op == OP_ASSIGN_FINAL) {
				warnx("%s:%d: %s key takes '=' or '+=' operator.",
					state->cur_file->name, yyget_lineno(scanner), key2str(pair->key));
				warning_update_retcode(state);
				pair->op = OP_ASSIGN;
			}
			break;
		case KEY_TAG:
			if (pair->attr)
				rule_log_invalid_attr(scanner, state, pair->key);

			if (pair->op == OP_ASSIGN_FINAL) {
				warnx("%s:%d: %s key takes '==', '!=', '=', or '+=' operator.",
					state->cur_file->name, yyget_lineno(scanner), key2str(pair->key));
				warning_update_retcode(state);
				pair->op = OP_ASSIGN;
			}
			break;
		case KEY_TAGS:
			if (pair->attr)
				rule_log_invalid_attr(scanner, state, pair->key);
			break;
		case KEY_TEST:
			if (!is_match)
				rule_log_invalid_op(scanner, state, pair->key, pair->op);
			break;
		default:
			warnx("%s:%d: unknown key \"%s\".",
				state->cur_file->name,
				yyget_lineno(scanner),
				key2str(pair->key));
			warning_update_retcode(state);
			return;
	}

	check_match_conditions(scanner, state, pair);

	//if (pair->attr)
	//	printf("(<%s> <%s> <%s> <%s>) ",
	//		key2str(pair->key), pair->attr->string, op2str(pair->op), pair->value->string);
	//else
	//	printf("(<%s> <%s> <%s>) ",
	//		key2str(pair->key), op2str(pair->op), pair->value->string);
}
%}

%language "C"
%debug
%define parse.error verbose

/* Pure yylex.  */
%define api.pure
%lex-param { void *scanner }

/* Pure yyparse.  */
%parse-param { void *scanner }
%parse-param { struct rules_state *state }

%token KEY ATTR OPERATION VALUE COMMA EOL ERROR

%union {
	int   num;
	struct string *string;
}

%type <string> ATTR
%type <string> VALUE
%type <num> KEY
%type <num> OPERATION

%start input

%%

input		:
		| input line
		;
line		: EOL
		| ruleline
		;
ruleline	: pairs EOL
		{
			state->global_rule_nr++;
			state->cur_file->rules_nr++;
			state->cur_rule = NULL;
			//printf("\n");
		}
		;
pairs		: pairs COMMA pair
		| pair
		;
pair		: KEY OPERATION VALUE
		{
			struct rule *rule = get_rule(scanner, state);
			struct rule_pair *pair = get_pair(rule);

			pair->key   = $1;
			pair->attr  = NULL;
			pair->op    = $2;
			pair->value = $3;

			process_token(scanner, state, pair);
		}
		| KEY ATTR OPERATION VALUE
		{
			struct rule *rule = get_rule(scanner, state);
			struct rule_pair *pair = get_pair(rule);

			pair->key   = $1;
			pair->attr  = $2;
			pair->op    = $3;
			pair->value = $4;

			process_token(scanner, state, pair);
		}
		;

%%

static void rules_parse(FILE *fp, struct rules_state *state)
{
	yyscan_t scanner;

	yylex_init(&scanner);
	yyset_in(fp, scanner);
	while (yyparse(scanner, state));
	yylex_destroy(scanner);
}

int rules_readfile(char *filename, struct rules_state *state)
{
	struct stat st;
	struct rule_file *rule_file;
	int ret = -1;
	FILE *fp = NULL;

	if (stat(filename, &st) < 0) {
		warn("stat: %s", filename);
		goto end;
	}

	if (!S_ISREG(st.st_mode)) {
		warn("%s: not a regular file", filename);
		goto end;
	}

	if ((fp = fopen(filename, "r")) == NULL) {
		warn("fopen: %s", filename);
		goto end;
	}

	if (!(rule_file = calloc(1, sizeof(*rule_file)))) {
		warn("malloc(rule_file)");
		goto end;
	}

	rule_file->name = strdup(filename);
	rule_file->rules_nr = 0;

	state->cur_file = rule_file;
	state->cur_rule = NULL;

	list_add_tail(&rule_file->list, state->files);

	rules_parse(fp, state);
	ret = 0;
end:
	if (fp)
		fclose(fp);

	return ret;
}

int rules_readdir(const char *dir, struct rules_state *state)
{
	DIR *d;
	struct dirent *ent;
	int ret = 0;

	if (!(d = opendir(dir))) {
		warn("opendir: %s", dir);
		return -1;
	}

	while ((ent = readdir(d)) != NULL) {
		char filename[PATH_MAX];
		size_t len = strlen(ent->d_name);

		if ((len <= 6) || strcmp(ent->d_name + (len - 6), ".rules"))
			continue;

		if (sprintf(filename, "%s/%s", dir, ent->d_name) < 0) {
			warn("sprintf");
			continue;
		}

		if (rules_readfile(filename, state) < 0)
			ret = -1;
	}

	closedir(d);
	return ret;
}

// vim: tw=200
