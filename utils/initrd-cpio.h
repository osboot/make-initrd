#ifndef INITRD_CPIO_H
#define INITRD_CPIO_H

#include "initrd-common.h"

enum cpio_type {
	CPIO_UNKNOWN = 0,
	CPIO_ARCHIVE,
	CPIO_BOOTCONFIG,
};

struct cpio {
	enum cpio_type type;
	const char *compress;

	unsigned char *raw;
	unsigned long size;

	struct list_tail *headers;
};

struct cpio_header {
	unsigned long ino, major, minor, rmajor, rminor, nlink;
	mode_t mode;
	time_t mtime;
	unsigned long body_len, name_len;
	uid_t uid;
	gid_t gid;
	unsigned rdev;
	char *name;
	char *body;
};

unsigned long read_cpio(struct cpio *archive);
void cpio_free(struct cpio *archive);

#include <stdio.h>

unsigned long write_cpio(struct cpio_header *data, unsigned long offset, FILE *output);
void write_trailer(unsigned long offset, FILE *output);

#endif /* INITRD_CPIO_H */
