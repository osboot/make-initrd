#ifndef INITRD_CPIO_H
#define INITRD_CPIO_H

#include "initrd-common.h"

struct cpio {
	const char *compress;

	unsigned char *raw;
	unsigned long size;

	struct list_tail *headers;
};

struct cpio_header {
	unsigned long ino, major, minor, nlink;
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


#endif /* INITRD_CPIO_H */
