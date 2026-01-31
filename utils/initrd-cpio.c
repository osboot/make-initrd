#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <err.h>
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

#define CPIO_HDR_FIELD_SIZE 8
#define CPIO_HDR_N_FIELDS 13

#define MINORBITS 20
#define MINORMASK ((1U << MINORBITS) - 1)

#define MAJOR(dev) ((unsigned int) ((dev) >> MINORBITS))
#define MINOR(dev) ((unsigned int) ((dev) &MINORMASK))
#define MKDEV(ma, mi) (((ma) << MINORBITS) | (mi))

#define N_ALIGN(len) ((((len) + 1) & ~3UL) + 2)

static inline unsigned
new_encode_dev(dev_t dev)
{
	unsigned v     = 0xff;
	unsigned major = MAJOR(dev);
	unsigned minor = MINOR(dev);
	return (minor & 0xff) | (major << 8) | ((minor & ~v) << 12);
}

static int
parse_header(const unsigned char *s, struct cpio_header *h)
{
	unsigned long parsed[CPIO_HDR_N_FIELDS];
	char hexbuf[CPIO_HDR_FIELD_SIZE + 1];
	int i;

	hexbuf[CPIO_HDR_FIELD_SIZE] = '\0';

	s += CPIO_FORMAT_LENGTH;
	i = 0;

	while (i < CPIO_HDR_N_FIELDS) {
		memcpy(hexbuf, s, CPIO_HDR_FIELD_SIZE);
		parsed[i] = strtoul(hexbuf, NULL, 16);

		s += CPIO_HDR_FIELD_SIZE;
		i++;
	}

	h->ino      = parsed[0];
	h->mode     = (mode_t) parsed[1];
	h->uid      = (uid_t) parsed[2];
	h->gid      = (gid_t) parsed[3];
	h->nlink    = parsed[4];
	h->mtime    = (time_t) parsed[5];
	h->body_len = parsed[6];
	h->major    = parsed[7];
	h->minor    = parsed[8];
	h->rmajor   = parsed[9];
	h->rminor   = parsed[10];
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

	while (offset < a->size) {
		if (a->size < (CPIO_FORMAT_LENGTH + CPIO_HEADER_SIZE))
			errx(EXIT_FAILURE, "archive less than header");

		if (memcmp(a->raw + offset, CPIO_FORMAT_OLDASCII, CPIO_FORMAT_LENGTH) == 0)
			errx(EXIT_FAILURE, "incorrect cpio method used: use -H newc option");

		if (memcmp(a->raw + offset, CPIO_FORMAT_NEWASCII, CPIO_FORMAT_LENGTH))
			errx(EXIT_FAILURE, "no cpio magic");

		e = list_append(&a->headers, sizeof(struct cpio_header));
		if (e == NULL)
			err(EXIT_FAILURE, "unable to add element to list");

		if (offset + CPIO_HEADER_SIZE > a->size)
			errx(EXIT_FAILURE, "cpio header exceeds archive size");

		h = e->data;

		parse_header(a->raw + offset, h);

		offset += CPIO_HEADER_SIZE;
		offset += N_ALIGN(h->name_len) + h->body_len;
		offset = (offset + 3) & ~3UL;

		if (offset > a->size)
			errx(EXIT_FAILURE, "cpio entry exceeds archive size");

		if (h->name && h->name_len >= strlen(CPIO_TRAILER) + 1 &&
		    !memcmp(h->name, CPIO_TRAILER, strlen(CPIO_TRAILER))) {
			list_shift(&a->headers);
			while (offset % 512) {
				offset++;
			}
			break;
		}
	}

	return offset;
}

static unsigned long
push_hdr(const char *s, unsigned long offset, FILE *output)
{
	fputs(s, output);
	offset += CPIO_HEADER_SIZE;
	return offset;
}

static unsigned long
push_rest(const char *name, unsigned long offset, FILE *output)
{
	size_t name_len = strlen(name) + 1;
	size_t tmp_ofs;

	fputs(name, output);
	fputc(0, output);
	offset += name_len;

	tmp_ofs = CPIO_HEADER_SIZE + name_len;
	while (tmp_ofs & 3) {
		fputc(0, output);
		offset++;
		tmp_ofs++;
	}
	return offset;
}

static unsigned long
push_string(const char *name, unsigned long offset, FILE *output)
{
	size_t name_len = strlen(name) + 1;

	fputs(name, output);
	fputc(0, output);
	offset += name_len;
	return offset;
}

static unsigned long
push_pad(unsigned long offset, FILE *output)
{
	while (offset & 3) {
		fputc(0, output);
		offset++;
	}
	return offset;
}

unsigned long
write_cpio(struct cpio_header *data, unsigned long offset, FILE *output)
{
	char s[256];

	sprintf(s, "%s%08lX%08X%08lX%08lX%08lX%08lX"
	        "%08lX%08lX%08lX%08lX%08lX%08lX%08X",
	        CPIO_FORMAT_NEWASCII, /* magic */
	        data->ino,            /* ino */
	        data->mode,           /* mode */
	        (long) data->uid,     /* uid */
	        (long) data->gid,     /* gid */
	        data->nlink,          /* nlink */
	        data->mtime,          /* mtime */

	        data->body_len, /* filesize */
	        data->major,    /* major */
	        data->minor,    /* minor */
	        data->rmajor,   /* rmajor */
	        data->rminor,   /* rminor */
	        data->name_len, /* namesize */
	        0);             /* chksum */

	offset = push_hdr(s, offset, output);

	switch (data->mode & S_IFMT) {
		case S_IFBLK:
		case S_IFCHR:
		case S_IFDIR:
		case S_IFIFO:
		case S_IFSOCK:
			offset = push_rest(data->name, offset, output);
			return offset;
	}

	offset = push_string(data->name, offset, output);
	offset = push_pad(offset, output);

	if (data->body_len) {
		fwrite(data->body, data->body_len, 1, output);
		offset += data->body_len;
		offset = push_pad(offset, output);
	}

	return offset;
}

void
write_trailer(unsigned long offset, FILE *output)
{
	char s[256];
	const char name[] = "TRAILER!!!";

	sprintf(s, "%s%08X%08X%08lX%08lX%08X%08lX"
	        "%08X%08X%08X%08X%08X%08X%08X",
	        CPIO_FORMAT_NEWASCII,        /* magic */
	        0,                           /* ino */
	        0,                           /* mode */
	        (long) 0,                    /* uid */
	        (long) 0,                    /* gid */
	        1,                           /* nlink */
	        (long) 0,                    /* mtime */
	        0,                           /* filesize */
	        0,                           /* major */
	        0,                           /* minor */
	        0,                           /* rmajor */
	        0,                           /* rminor */
	        (unsigned) strlen(name) + 1, /* namesize */
	        0);                          /* chksum */

	offset = push_hdr(s, offset, output);
	offset = push_rest(name, offset, output);

	while (offset % 512) {
		fputc(0, output);
		offset++;
	}
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
