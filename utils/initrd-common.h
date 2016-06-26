#ifndef INITRD_COMMON_H
#define INITRD_COMMON_H

#include <sys/types.h>

struct list_tail {
	struct list_tail *next;
	void *data;
};

struct list_tail *list_append(struct list_tail **head, size_t size);
void list_shift(struct list_tail **head);
void list_free(struct list_tail *head);

#endif /* INITRD_COMMON_H */
