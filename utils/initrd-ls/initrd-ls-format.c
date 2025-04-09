// SPDX-License-Identifier: GPL-3.0-or-later
#define _GNU_SOURCE

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <err.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>

#include "initrd-ls.h"

#define STAT_ISSET(mode, mask) (((mode) &mask) == mask)

extern int opts;

static int max_nlinks = 1;
static int max_size   = 1;
static int max_uid    = 1;
static int max_gid    = 1;
static int max_min    = 1;
static int max_maj    = 1;

int
preformat(struct cpio_header *h)
{
	int bytes;

	bytes = snprintf(NULL, 0, "%ju", (uintmax_t) h->nlink);
	if (bytes > max_nlinks)
		max_nlinks = bytes;

	bytes = snprintf(NULL, 0, "%ju", (uintmax_t) h->uid);
	if (bytes > max_uid)
		max_uid = bytes;

	bytes = snprintf(NULL, 0, "%ju", (uintmax_t) h->gid);
	if (bytes > max_gid)
		max_gid = bytes;

	if (S_ISCHR(h->mode) || S_ISBLK(h->mode)) {
		bytes = snprintf(NULL, 0, "%u", major(h->rdev));
		if (bytes > max_maj)
			max_maj = bytes;

		bytes = snprintf(NULL, 0, "%u", minor(h->rdev));
		if (bytes > max_min)
			max_min = bytes;

		if ((max_maj + max_min + 1) > max_size)
			max_size = max_maj + max_min + 1;
	} else {
		bytes = snprintf(NULL, 0, "%ju", (uintmax_t) h->body_len);
		if (bytes > max_size)
			max_size = bytes;
	}
	return 0;
}

int
show_header(struct cpio_header *h)
{
	char *fmt;
	int rc;

	char timbuf[80];
	struct tm lt;

	if (!(opts & SHOW_NO_MTIME)) {
		localtime_r(&h->mtime, &lt);
		strftime(timbuf, sizeof(timbuf), "%b %d %T %Y ", &lt);
	} else {
		timbuf[0] = '\0';
	}

	switch (h->mode & S_IFMT) {
		case S_IFBLK:
			putchar('b');
			break;
		case S_IFCHR:
			putchar('c');
			break;
		case S_IFDIR:
			putchar('d');
			break;
		case S_IFIFO:
			putchar('p');
			break;
		case S_IFLNK:
			putchar('l');
			break;
		case S_IFSOCK:
			putchar('s');
			break;
		case S_IFREG:
			putchar('-');
			break;
		default:
			putchar('?');
			break;
	}
	putchar(STAT_ISSET(h->mode, S_IRUSR) ? 'r' : '-');
	putchar(STAT_ISSET(h->mode, S_IWUSR) ? 'w' : '-');

	!STAT_ISSET(h->mode, S_ISUID) ? putchar(STAT_ISSET(h->mode, S_IXUSR) ? 'x' : '-') : putchar('S');

	putchar(STAT_ISSET(h->mode, S_IRGRP) ? 'r' : '-');
	putchar(STAT_ISSET(h->mode, S_IWGRP) ? 'w' : '-');

	!STAT_ISSET(h->mode, S_ISGID) ? putchar(STAT_ISSET(h->mode, S_IXGRP) ? 'x' : '-') : putchar('S');

	putchar(STAT_ISSET(h->mode, S_IROTH) ? 'r' : '-');
	putchar(STAT_ISSET(h->mode, S_IWOTH) ? 'w' : '-');

	!STAT_ISSET(h->mode, S_ISVTX) ? putchar(STAT_ISSET(h->mode, S_IXOTH) ? 'x' : '-') : putchar(S_ISDIR(h->mode) ? 't' : 'T');

	if (S_ISCHR(h->mode) || S_ISBLK(h->mode)) {
		rc = asprintf(&fmt, " %%%dju %%%dju %%%dju %%%du,%%%du %%s%%s",
		              max_nlinks, max_uid, max_gid, max_size - max_maj - max_min, max_min);
		if (rc == -1)
			err(EXIT_FAILURE, "ERROR: asprintf");

		fprintf(stdout, fmt,
		        (uintmax_t) h->nlink,
		        (uintmax_t) h->uid,
		        (uintmax_t) h->gid,
		        major(h->rdev),
		        minor(h->rdev),
		        timbuf,
		        h->name);
	} else {
		rc = asprintf(&fmt, " %%%dju %%%dju %%%dju %%%dju %%s%%s",
		              max_nlinks, max_uid, max_gid, max_size);
		if (rc == -1)
			err(EXIT_FAILURE, "ERROR: asprintf");

		fprintf(stdout, fmt,
		        (uintmax_t) h->nlink,
		        (uintmax_t) h->uid,
		        (uintmax_t) h->gid,
		        (uintmax_t) h->body_len,
		        timbuf,
		        h->name);
	}
	free(fmt);

	if (S_ISLNK(h->mode)) {
		fprintf(stdout, " -> %s", h->body);
	}

	putchar('\n');
	return 0;
}
