/* SPDX-License-Identifier: GPL-2.0-or-later */
#include <stdlib.h>
#include <stdbool.h>
#include <err.h>
#include "udev-rules.h"

struct rule_goto_label *get_goto(struct rules_state *state)
{
	struct rule_goto_label *new;

	if (!(new = malloc(sizeof(*new))))
		err(1, "malloc(goto)");

	list_add_tail(&new->list, state->gotos);
	return new;
}

struct rule_goto_label *get_label(struct rules_state *state)
{
	struct rule_goto_label *new;

	if (!(new = malloc(sizeof(*new))))
		err(1, "malloc(label)");

	list_add_tail(&new->list, state->labels);
	return new;
}

void free_goto_label(struct list_head *head)
{
	while (!list_empty(head)) {
		struct rule_goto_label *ent = list_first_entry(head, struct rule_goto_label, list);
		list_del(&ent->list);
		free(ent);
	}
}

static inline char *rec_str(struct rule_goto_label *rec)
{
	return rec->pair->value->string;
}

static inline int rec_order(struct rule_goto_label *rec)
{
	return rec->pair->rule->line_nr;
}

void check_goto_label(struct rules_state *state)
{
	struct rule_goto_label *r_goto;
	struct rule_goto_label *r_label;
	bool found;

	list_for_each_entry(r_goto, state->gotos, list) {
		found = false;

		list_for_each_entry(r_label, state->labels, list) {
			if (strcmp(rec_str(r_goto), rec_str(r_label)))
				continue;

			if (pair_file(r_goto->pair) != pair_file(r_label->pair))
				continue;

			if (rec_order(r_label) > rec_order(r_goto)) {
				found = true;
				break;
			}
		}

		if (state->warning[W_MISSING_LABEL] && !found) {
			warnx("%s:%d:%d: GOTO=\"%s\" has no matching label [-W%s]",
			      pair_fname(r_goto->pair), key_line(r_goto->pair), key_column(r_goto->pair),
			      rec_str(r_goto),
			      warning_str[W_MISSING_LABEL]);
			warning_update_retcode(state, W_MISSING_LABEL);
		}
	}

	if (!state->warning[W_UNUSED_LABELS])
		return;

	list_for_each_entry(r_label, state->labels, list) {
		found = false;

		list_for_each_entry(r_goto, state->gotos, list) {
			if (pair_file(r_goto->pair) != pair_file(r_label->pair))
				continue;

			if (strcmp(rec_str(r_goto), rec_str(r_label)))
				continue;

			if (rec_order(r_goto) >= rec_order(r_label))
				break;

			found = true;
			break;
		}

		if (!found) {
			warnx("%s:%d:%d: LABEL=\"%s\" takes no effect [-W%s]",
			      pair_fname(r_label->pair), key_line(r_label->pair), key_column(r_label->pair),
			      rec_str(r_label),
			      warning_str[W_UNUSED_LABELS]);
			warning_update_retcode(state, W_UNUSED_LABELS);
		}
	}
}
