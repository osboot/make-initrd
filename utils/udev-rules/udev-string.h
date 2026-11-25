// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef __UDEV_STRING_H__
#define __UDEV_STRING_H__

#include <stdlib.h>
#include <string.h>

#include "compiler.h"

struct string {
	char *string;
	size_t length;
	size_t bufsz;
};

struct string *new_string(void) _returns_nonnull_ _must_check_;
void free_string(struct string *s);
void resize_string(struct string *s, size_t size) _rdwr_ptr_(1);
char *string_append_str(struct string *s, const char *str, size_t n)
	_rdwr_ptr_(1) _rdonly_ptr_(2, 3);
char *string_append_chr(struct string *s, const char c) _rdwr_ptr_(1);

#endif /* __UDEV_STRING_H__ */
