// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef INITRD_PARSE_H
#define INITRD_PARSE_H

#include "initrd-common.h"

struct stream {
	short allocated;
	unsigned char *addr;
	unsigned long size;
};

struct result {
	struct list_tail *streams;
	struct list_tail *cpios;
};

void read_stream(const char *compress, struct stream *stream, struct result *res);
void free_streams(struct list_tail *streams);
void free_cpios(struct list_tail *cpios);

#endif /* INITRD_PARSE_H */
