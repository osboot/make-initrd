/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef __UDEV_STRING_H__
#define __UDEV_STRING_H__

#include <stdlib.h>
#include <string.h>

struct string {
	char *string;
	size_t length;
	size_t bufsz;
};

struct string *new_string(void);
void free_string(struct string *s);
void resize_string(struct string *s, size_t size);
char *string_append_str(struct string *s, const char *str, size_t n);
char *string_append_chr(struct string *s, const char c);

#endif /* __UDEV_STRING_H__ */
