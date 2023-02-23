/* SPDX-License-Identifier: GPL-2.0-or-later */

#include <err.h>

#include "udev-string.h"

#define STRING_ALLOC_SIZE 16

struct string *new_string(void)
{
	struct string *s;

	if (!(s = malloc(sizeof(*s))))
		err(1, "malloc");

	s->bufsz = STRING_ALLOC_SIZE;

	if (!(s->string = malloc(s->bufsz)))
		err(1, "malloc");

	s->string[0] = '\0';
	s->length = 0;

	return s;
}

void free_string(struct string *s)
{
	if (s) {
		free(s->string);
		free(s);
	}
}

void resize_string(struct string *s, size_t size)
{
	if (size > s->bufsz) {
		s->bufsz = (size / STRING_ALLOC_SIZE + 1) * STRING_ALLOC_SIZE * 2;
		s->string = realloc(s->string, s->bufsz);
		if (!s->string)
			err(1, "realloc");
	}
}

char *string_append_str(struct string *s, const char *str, size_t n)
{
	size_t len = s->length + n;
	resize_string(s, len + 1);

	strncpy(s->string + s->length, str, n);
	s->string[len] = '\0';

	s->length = len;

	return s->string;
}

char *string_append_chr(struct string *s, const char c)
{
	resize_string(s, s->length + 2);

	s->string[s->length++] = c;
	s->string[s->length] = '\0';

	return s->string;
}
