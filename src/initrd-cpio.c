#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <error.h>
#include <libgen.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

//#include "initrd.h"
#include "initrd-cpio.h"

#define CPIO_HEADER_SIZE 110
#define CPIO_TRAILER "TRAILER!!!"

#define CPIO_FORMAT_NEWASCII "070701"
#define CPIO_FORMAT_CRCASCII "070702"
#define CPIO_FORMAT_OLDASCII "070707"
#define CPIO_FORMAT_LENGTH 6

#define MINORBITS	20
#define MINORMASK	((1U << MINORBITS) - 1)

#define MAJOR(dev)	((unsigned int) ((dev) >> MINORBITS))
#define MINOR(dev)	((unsigned int) ((dev) & MINORMASK))
#define MKDEV(ma,mi)	(((ma) << MINORBITS) | (mi))

#define N_ALIGN(len) ((((len) + 1) & ~3) + 2)

static inline unsigned new_encode_dev(dev_t dev)
{
	unsigned major = MAJOR(dev);
	unsigned minor = MINOR(dev);
	return (minor & 0xff) | (major << 8) | ((minor & ~0xff) << 12);
}

static int
parse_header(const unsigned char *s, struct cpio_header *h)
{
	unsigned long parsed[12];
	char buf[9];
	int i;

	buf[8] = '\0';
	for (i = 0, s += 6; i < 12; i++, s += 8) {
		memcpy(buf, s, 8);
		parsed[i] = strtoul(buf, NULL, 16);
	}
	s += 8;

	h->ino      = parsed[0];
	h->mode     = (mode_t) parsed[1];
	h->uid      = (uid_t) parsed[2];
	h->gid      = (gid_t) parsed[3];
	h->nlink    = parsed[4];
	h->mtime    = (time_t) parsed[5];
	h->body_len = parsed[6];
	h->major    = parsed[7];
	h->minor    = parsed[8];
	h->rdev     = new_encode_dev(MKDEV(parsed[9], parsed[10]));
	h->name_len = parsed[11];
	h->name     = (char *) s;
	h->body     = (char *) s + N_ALIGN(h->name_len);

	return 1;
}

unsigned long
read_cpio(struct cpio *a)
{
	struct list_tail *e;
	struct cpio_header *h;
	unsigned long offset = 0;

	while(offset < a->size) {
		if (a->size < (CPIO_FORMAT_LENGTH + CPIO_HEADER_SIZE))
			error(EXIT_FAILURE, 0, "archive less than header");

		if (memcmp(a->raw + offset, CPIO_FORMAT_OLDASCII, CPIO_FORMAT_LENGTH) == 0)
			error(EXIT_FAILURE, 0, "incorrect cpio method used: use -H newc option");

		if (memcmp(a->raw + offset, CPIO_FORMAT_NEWASCII, CPIO_FORMAT_LENGTH))
			error(EXIT_FAILURE, 0, "no cpio magic");

		e = list_append(&a->headers, sizeof(struct cpio_header));
		if (e == NULL)
			error(EXIT_FAILURE, errno, "unable to add element to list");
		h = e->data;

		parse_header(a->raw + offset, h);
		offset += CPIO_HEADER_SIZE;

		offset += N_ALIGN(h->name_len) + h->body_len;
		offset = (offset + 3) & ~3;

		if (h->name && !memcmp(h->name, CPIO_TRAILER, strlen(CPIO_TRAILER))) {
			list_shift(&a->headers);
			while (offset % 512) {
				offset++;
			}
			break;
		}
	}

	return offset;
}

void
cpio_free(struct cpio *c)
{
	list_free(c->headers);

	c->compress = NULL;
	c->raw      = NULL;
	c->size     = 0;
	c->headers  = NULL;
}
