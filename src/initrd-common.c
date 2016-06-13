#include <stdlib.h>

#include "initrd-common.h"

struct list_tail *
list_append(struct list_tail **head, size_t size)
{
	struct list_tail *e, *l;

	if ((e = malloc(sizeof(struct list_tail))) == NULL)
		goto err;

	e->next = NULL;
	e->data = NULL;

	if (size > 0) {
		if ((e->data = malloc(size)) == NULL)
			goto err;
	}

	if (*head != NULL) {
		l = *head;
		while (l->next != NULL) {
			l = l->next;
		}
		l->next = e;
	} else {
		*head = e;
	}

	return e;
err:
	if (e != NULL) {
		if (e->data != NULL)
			free(e->data);
		free(e);
	}

	return NULL;
}

void
list_shift(struct list_tail **head)
{
	struct list_tail *p, *l;

	p = l = *head;
	while (l && l->next) {
		p = l;
		l = l->next;
	}

	if (l) {
		if (l->data)
			free(l->data);
		free(l);
	}

	if (p)
		p->next = NULL;
}

void
list_free(struct list_tail *head)
{
	struct list_tail *l, *n;

	l = head;
	while(l) {
		if (l->data)
			free(l->data);
		n = l->next;
		free(l);
		l = n;
	}
}
