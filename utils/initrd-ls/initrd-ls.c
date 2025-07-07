// SPDX-License-Identifier: GPL-3.0-or-later

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <err.h>
#include <libgen.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "initrd-common.h"
#include "initrd-cpio.h"
#include "initrd-decompress.h"
#include "initrd-parse.h"
#include "initrd-ls.h"

int opts = 0;

static const char cmdopts_s[] = "bnCVh";
static const struct option cmdopts[] = {
	{ "brief", no_argument, 0, 'b' },
	{ "name", no_argument, 0, 'n' },
	{ "no-mtime", no_argument, 0, 3 },
	{ "compression", no_argument, 0, 'C' },
	{ "version", no_argument, 0, 'V' },
	{ "help", no_argument, 0, 'h' },
	{ NULL, 0, 0, 0 }
};

static void __attribute__((noreturn))
print_help(const char *progname)
{
	printf("Usage: %s [options] initramfs\n"
	       "\n"
	       "Displays initramfs contents in a format similar to ls command.\n"
	       "If initramfs contains more than one cpio archive, utility will show all of them.\n"
	       "Archives can be compressed. In this case, utility will take a look inside.\n"
	       "\n"
	       "Options:\n"
	       "   --no-mtime          Hide modification time;\n"
	       "   -b, --brief         Show only brief information about archive parts;\n"
	       "   -n, --name          Show only filenames;\n"
	       "   -C, --compression   Show compression method for each archive;\n"
	       "   -V, --version       Show version of program and exit;\n"
	       "   -h, --help          Show this text and exit.\n"
	       "\n",
	       progname);
	exit(EXIT_SUCCESS);
}

static void __attribute__((noreturn))
print_version(const char *progname)
{
	printf("%s version " PACKAGE_VERSION "\n"
	       "Written by Alexey Gladkov <gladkov.alexey@gmail.com>\n"
	       "\n"
	       "Copyright (C) 2016-2024  Alexey Gladkov <gladkov.alexey@gmail.com>\n"
	       "This is free software; see the source for copying conditions.  There is NO\n"
	       "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
	       "\n",
	       progname);
	exit(EXIT_SUCCESS);
}

int
main(int argc, char **argv)
{
	int c, fd;
	struct stat st;

	while ((c = getopt_long(argc, argv, cmdopts_s, cmdopts, NULL)) != -1) {
		switch (c) {
			case 3:
				opts ^= SHOW_NO_MTIME;
				break;
			case 'b':
				opts ^= SHOW_BRIEF;
				break;
			case 'n':
				opts ^= SHOW_NAME_ONLY;
				break;
			case 'C':
				opts ^= SHOW_COMPRESSION;
				break;
			case 'V':
				print_version(basename(argv[0]));
			case 'h':
				print_help(basename(argv[0]));
			default:
				exit(EXIT_FAILURE);
		}
	}

	if (optind >= argc)
		errx(EXIT_FAILURE, "ERROR: Missing initrd file");

	errno = 0;
	if (stat(argv[optind], &st) == -1) {
		if (errno == ENOENT)
			errx(EXIT_FAILURE, "ERROR: initrd does not exist: %s", argv[optind]);
		err(EXIT_FAILURE, "ERROR: stat: %s", argv[optind]);
	}

	if ((fd = open(argv[optind], O_RDONLY)) == -1)
		err(EXIT_FAILURE, "ERROR: open: %s", argv[optind]);

	unsigned char *addr = mmap(NULL, (size_t) st.st_size, PROT_READ, MAP_PRIVATE | MAP_POPULATE, fd, 0);

	if (addr == MAP_FAILED)
		err(EXIT_FAILURE, "ERROR: mmap");

	unsigned long n_cpio = 0;
	struct stream *s;
	struct list_tail *l, *h;
	struct result res;

	res.streams = NULL;
	res.cpios   = NULL;

	l = list_append(&res.streams, sizeof(struct stream));
	if (l == NULL)
		err(EXIT_FAILURE, "unable to add element to list");
	s = l->data;

	s->addr      = addr;
	s->size      = (unsigned long) st.st_size;
	s->allocated = 0;

	read_stream("raw", s, &res);

	int bytes;
	int max_compress_name = 3;
	char *fmt             = NULL;
	struct cpio *part     = NULL;

	l = res.cpios;
	while (l) {
		part = (struct cpio *) l->data;

		if ((opts & SHOW_BRIEF) || part->type != CPIO_ARCHIVE) {
			l = l->next;
			n_cpio++;
			continue;
		}
		if (opts & SHOW_COMPRESSION) {
			bytes = snprintf(NULL, 0, "%s", part->compress);
			if (bytes > max_compress_name)
				max_compress_name = bytes;
		}
		h = part->headers;
		while (h) {
			preformat(h->data);
			h = h->next;
		}
		l = l->next;
		n_cpio++;
	}

	bytes = snprintf(NULL, 0, "%ju", n_cpio);

	c = (opts & SHOW_COMPRESSION)
		? asprintf(&fmt, "%%%dd %%%ds ", bytes, max_compress_name)
		: asprintf(&fmt, "%%%dd ", bytes);

	if (c == -1)
		err(EXIT_FAILURE, "ERROR: asprintf");

	c = 1;
	l = res.cpios;
	while (l) {
		part = (struct cpio *) l->data;

		if (opts & SHOW_BRIEF) {
			fprintf(stdout, "%d\t", c);

			switch (part->type) {
				case CPIO_ARCHIVE:
					if (strcmp("raw", part->compress))
						fprintf(stdout, "%s compressed ", part->compress);
					fprintf(stdout, "cpio archive");
					break;
				case CPIO_BOOTCONFIG:
					fprintf(stdout, "bootconfig");
					break;
				case CPIO_UNKNOWN:
					fprintf(stdout, "unknown");
					break;
			}

			fprintf(stdout, ", size %ld bytes\n", part->size);

			l = l->next;
			c++;
			continue;
		}
		if (part->type != CPIO_ARCHIVE) {
			l = l->next;
			n_cpio++;
			continue;
		}
		h = part->headers;
		while (h) {
			(opts & SHOW_COMPRESSION)
				? fprintf(stdout, fmt, c, part->compress)
				: fprintf(stdout, fmt, c);
			(opts & SHOW_NAME_ONLY)
				? fprintf(stdout, "%s\n", ((struct cpio_header *) h->data)->name)
				: show_header(h->data);
			h = h->next;
		}
		l = l->next;
		c++;
	}

	free(fmt);

	free_cpios(res.cpios);
	free_streams(res.streams);

	munmap(addr, (size_t) st.st_size);

	return EXIT_SUCCESS;
}
