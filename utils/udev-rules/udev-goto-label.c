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

void check_goto_label(struct rules_state *state)
{
	struct rule_goto_label *r_goto;
	struct rule_goto_label *r_label;
	bool found;

	list_for_each_entry(r_goto, state->gotos, list) {
		found = false;

		list_for_each_entry(r_label, state->labels, list) {
			if (strcmp(r_goto->name->string, r_label->name->string))
				continue;

			if (r_goto->rule->file != r_label->rule->file)
				continue;

			if (r_label->rule->global_order > r_goto->rule->global_order) {
				found = true;
				break;
			}
		}

		if (state->warning[W_MISSING_LABEL] && !found) {
			warnx("%s:%d: GOTO=\"%s\" has no matching label [-W%s]",
			      r_goto->rule->file->name,
			      r_goto->rule->line_nr,
			      r_goto->name->string,
			      warning_str[W_MISSING_LABEL]);
			warning_update_retcode(state);
		}
	}

	if (!state->warning[W_UNUSED_LABELS])
		return;

	list_for_each_entry(r_label, state->labels, list) {
		found = false;

		list_for_each_entry(r_goto, state->gotos, list) {
			if (r_goto->rule->file != r_label->rule->file)
				continue;

			if (strcmp(r_goto->name->string, r_label->name->string))
				continue;

			if (r_goto->rule->global_order >= r_label->rule->global_order)
				break;

			found = true;
			break;
		}

		if (!found) {
			warnx("%s:%d: LABEL=\"%s\" takes no effect [-W%s]",
			      r_label->rule->file->name,
			      r_label->rule->line_nr,
			      r_label->name->string,
			      warning_str[W_UNUSED_LABELS]);
			warning_update_retcode(state);
		}
	}
}
