/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef __UDEV_RULES_H__
#define __UDEV_RULES_H__

typedef enum {
	OP_MATCH,        /* == */
	OP_NOMATCH,      /* != */
	OP_ADD,          /* += */
	OP_REMOVE,       /* -= */
	OP_ASSIGN,       /* = */
	OP_ASSIGN_FINAL, /* := */
	_OP_TYPE_MAX,
} rule_op_t;

#define _OP_TYPE_IS_ACTION OP_ADD

static const char *const rule_op_names[_OP_TYPE_MAX] = {
	[OP_ASSIGN]       = "=",
	[OP_ASSIGN_FINAL] = ":=",
	[OP_ADD]          = "+=",
	[OP_REMOVE]       = "-=",
	[OP_MATCH]        = "==",
	[OP_NOMATCH]      = "!=",
};

typedef enum {
	KEY_ACTION,
	KEY_ATTR,
	KEY_ATTRS,
	KEY_CONST,
	KEY_DEVPATH,
	KEY_DRIVER,
	KEY_DRIVERS,
	KEY_ENV,
	KEY_GOTO,
	KEY_GROUP,
	KEY_IMPORT,
	KEY_KERNEL,
	KEY_KERNELS,
	KEY_LABEL,
	KEY_MODE,
	KEY_NAME,
	KEY_OPTIONS,
	KEY_OWNER,
	KEY_PROGRAM,
	KEY_RESULT,
	KEY_RUN,
	KEY_SECLABEL,
	KEY_SUBSYSTEM,
	KEY_SUBSYSTEMS,
	KEY_SYMLINK,
	KEY_SYSCTL,
	KEY_TAG,
	KEY_TAGS,
	KEY_TEST,
	_KEY_TYPE_MAX,
} rule_key_t;

#define PAIR_OP_MATCH  0
#define PAIR_OP_ACTION _KEY_TYPE_MAX

static const char *const rule_key_names[_KEY_TYPE_MAX] = {
	[KEY_ACTION]     = "ACTION",
	[KEY_ATTRS]      = "ATTRS",
	[KEY_ATTR]       = "ATTR",
	[KEY_CONST]      = "CONST",
	[KEY_DEVPATH]    = "DEVPATH",
	[KEY_DRIVERS]    = "DRIVERS",
	[KEY_DRIVER]     = "DRIVER",
	[KEY_ENV]        = "ENV",
	[KEY_GOTO]       = "GOTO",
	[KEY_GROUP]      = "GROUP",
	[KEY_IMPORT]     = "IMPORT",
	[KEY_KERNELS]    = "KERNELS",
	[KEY_KERNEL]     = "KERNEL",
	[KEY_LABEL]      = "LABEL",
	[KEY_MODE]       = "MODE",
	[KEY_NAME]       = "NAME",
	[KEY_OPTIONS]    = "OPTIONS",
	[KEY_OWNER]      = "OWNER",
	[KEY_PROGRAM]    = "PROGRAM",
	[KEY_RESULT]     = "RESULT",
	[KEY_RUN]        = "RUN",
	[KEY_SECLABEL]   = "SECLABEL",
	[KEY_SUBSYSTEMS] = "SUBSYSTEMS",
	[KEY_SUBSYSTEM]  = "SUBSYSTEM",
	[KEY_SYMLINK]    = "SYMLINK",
	[KEY_SYSCTL]     = "SYSCTL",
	[KEY_TAGS]       = "TAGS",
	[KEY_TAG]        = "TAG",
	[KEY_TEST]       = "TEST",
};

#define key2str(pair) rule_key_names[pair->key]
#define op2str(pair)  rule_op_names[pair->op]

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

static const unsigned int rule_pair_prio[] = {
	KEY_LABEL      + PAIR_OP_ACTION,

	KEY_ACTION     + PAIR_OP_MATCH,
	KEY_DEVPATH    + PAIR_OP_MATCH,
	KEY_KERNEL     + PAIR_OP_MATCH,
	KEY_SYMLINK    + PAIR_OP_MATCH,
	KEY_NAME       + PAIR_OP_MATCH,
	KEY_ENV        + PAIR_OP_MATCH,
	KEY_CONST      + PAIR_OP_MATCH,
	KEY_TAG        + PAIR_OP_MATCH,
	KEY_SUBSYSTEM  + PAIR_OP_MATCH,
	KEY_DRIVER     + PAIR_OP_MATCH,
	KEY_ATTR       + PAIR_OP_MATCH,
	KEY_SYSCTL     + PAIR_OP_MATCH,

	KEY_KERNELS    + PAIR_OP_MATCH,
	KEY_SUBSYSTEMS + PAIR_OP_MATCH,
	KEY_DRIVERS    + PAIR_OP_MATCH,
	KEY_ATTRS      + PAIR_OP_MATCH,
	KEY_TAGS       + PAIR_OP_MATCH,

	KEY_TEST       + PAIR_OP_MATCH,
	KEY_PROGRAM    + PAIR_OP_MATCH,
	KEY_IMPORT     + PAIR_OP_MATCH,
	KEY_RESULT     + PAIR_OP_MATCH,

	KEY_OPTIONS    + PAIR_OP_ACTION,
	KEY_OWNER      + PAIR_OP_ACTION,
	KEY_GROUP      + PAIR_OP_ACTION,
	KEY_MODE       + PAIR_OP_ACTION,
	KEY_TAG        + PAIR_OP_ACTION,
	KEY_SECLABEL   + PAIR_OP_ACTION,
	KEY_ENV        + PAIR_OP_ACTION,
	KEY_NAME       + PAIR_OP_ACTION,
	KEY_SYMLINK    + PAIR_OP_ACTION,
	KEY_ATTR       + PAIR_OP_ACTION,
	KEY_SYSCTL     + PAIR_OP_ACTION,
	KEY_RUN        + PAIR_OP_ACTION,

	KEY_GOTO       + PAIR_OP_ACTION,
};

#include "list.h"
#include "udev-string.h"

struct rule_file {
	struct list_head list;
	char *name;
	int rules_nr;
};

struct pos {
	int line;
	int column;
};

struct rule;

struct rule_pair {
	struct list_head list;
	struct pos pos_key;
	struct pos pos_op;
	struct pos pos_value;
	rule_key_t key;
	rule_op_t op;
	struct string *attr;
	struct string *value;
	struct rule *rule;
};

struct rule {
	struct list_head list;
	struct list_head pairs;
	int line_nr;
	struct rule_file *file;
};

struct rule_goto_label {
	struct list_head list;
	struct rule_pair *pair;
};

enum {
	W_ERROR,
	W_MISSING_LABEL,
	W_UNUSED_LABELS,
	W_DUP_MATCH,
	W_CONFLICT_MATCH,
	W_INCOMPLETE_RULES,
	W_MULTI_LABEL,
	W_MULTI_GOTO,
	_W_TYPE_MAX,
};

extern const char *warning_str[_W_TYPE_MAX];

struct rules_state {
	struct list_head *files;
	struct list_head *rules;

	struct list_head *gotos;
	struct list_head *labels;

	int global_rule_nr;
	struct rule_file *cur_file;
	struct rule *cur_rule;

	int warning[_W_TYPE_MAX];
	int warning_summary[_W_TYPE_MAX];
	int show_external;

	int retcode;
};

static inline bool is_op_match(struct rule_pair *pair)
{
	return (pair->op < _OP_TYPE_IS_ACTION);
}

static inline int key_column(struct rule_pair *pair)
{
	return pair->pos_key.column;
}

static inline int op_column(struct rule_pair *pair)
{
	return pair->pos_op.column;
}

static inline int value_column(struct rule_pair *pair)
{
	return pair->pos_value.column;
}

static inline int key_line(struct rule_pair *pair)
{
	return pair->pos_key.line;
}

static inline int op_line(struct rule_pair *pair)
{
	return pair->pos_op.line;
}

static inline int value_line(struct rule_pair *pair)
{
	return pair->pos_value.line;
}

static inline struct rule_file *pair_file(struct rule_pair *pair)
{
	return pair->rule->file;
}

static inline char *pair_fname(struct rule_pair *pair)
{
	return pair_file(pair)->name;
}

extern int rules_readfile(char *filename, struct rules_state *state);
extern int rules_readdir(const char *dir, struct rules_state *state);

extern struct rule_goto_label *get_goto(struct rules_state *state);
extern struct rule_goto_label *get_label(struct rules_state *state);
extern void free_goto_label(struct list_head *head);
extern void check_goto_label(struct rules_state *state);

static inline void warning_update_retcode(struct rules_state *state, int warn)
{
	if (!state->retcode)
		state->retcode = state->warning[W_ERROR];
	if (warn > 0 && warn < _W_TYPE_MAX)
		state->warning_summary[warn]++;
}

static inline bool isempty(const char *a)
{
	return !a || a[0] == '\0';
}

#endif /* __UDEV_RULES_H__ */
