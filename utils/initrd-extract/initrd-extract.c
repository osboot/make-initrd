// SPDX-License-Identifier: GPL-3.0-or-later
#define _GNU_SOURCE

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

#include "initrd-common.h"
#include "initrd-cpio.h"
#include "initrd-decompress.h"
#include "initrd-parse.h"

int opts = 0;

static const char cmdopts_s[]        = "a:o:Vh";
static const struct option cmdopts[] = {
	{ "archive", required_argument, 0, 'a' },
	{ "output", required_argument, 0, 'o' },
	{ "version", no_argument, 0, 'V' },
	{ "help", no_argument, 0, 'h' },
	{ NULL, 0, 0, 0 }
};

static void __attribute__((noreturn))
print_help(const char *progname)
{
	printf("Usage: %s [options] initramfs\n"
	       "\n"
	       "Extracts part of initramfs\n"
	       "\n"
	       "Options:\n"
	       "   -a, --archive=<NUM>  Extract only specified initramfs;\n"
	       "   -o, --output=<FILE>  Write output to <FILE> instead of stdout;\n"
	       "   -V, --version        Show version of program and exit;\n"
	       "   -h, --help           Show this text and exit.\n"
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
	       "Copyright (C) 2017-2024  Alexey Gladkov <gladkov.alexey@gmail.com>\n"
	       "This is free software; see the source for copying conditions.  There is NO\n"
	       "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
	       "\n",
	       progname);
	exit(EXIT_SUCCESS);
}

static void __attribute__((noreturn))
bad_option_value(const char *optname, const char *value)
{
	errx(EXIT_FAILURE, "invalid value for \"%s\" option: %s", optname, value);
}

static int
str2int(const char *name, const char *value)
{
	char *p = 0;
	unsigned long n;

	if (!*value)
		bad_option_value(name, value);

	n = strtoul(value, &p, 10);
	if (!p || *p)
		bad_option_value(name, value);

	return (int) n;
}

int
main(int argc, char **argv)
{
	int c, fd, n_archive = 0;
	int option_index = 0;
	struct stat st;
	unsigned long offset;
	FILE *output = NULL;

	while ((c = getopt_long(argc, argv, cmdopts_s, cmdopts, &option_index)) != -1) {
		switch (c) {
			case 'a':
				n_archive = str2int(cmdopts[option_index].name, optarg);
				if (n_archive <= 0)
					bad_option_value(cmdopts[option_index].name, optarg);
				break;
			case 'o':
				if (output)
					fclose(output);
				if ((output = fopen(optarg, "w")) == NULL)
					err(EXIT_FAILURE, "ERROR: fopen: %s", optarg);
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
		errx(EXIT_FAILURE, "Missing initrd file");

	if (!output)
		output = stdout;

	if (stat(argv[optind], &st) == -1)
		err(EXIT_FAILURE, "ERROR: stat: %s", argv[optind]);

	if ((fd = open(argv[optind], O_RDONLY)) == -1)
		err(EXIT_FAILURE, "ERROR: open: %s", argv[optind]);

	unsigned char *addr = mmap(NULL, (size_t) st.st_size, PROT_READ, MAP_PRIVATE | MAP_POPULATE, fd, 0);

	if (addr == MAP_FAILED)
		err(EXIT_FAILURE, "ERROR: mmap");

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

	offset = 0;
	c      = 1;
	l      = res.cpios;
	while (l) {
		if (((struct cpio *) l->data)->type != CPIO_ARCHIVE) {
			l = l->next;
			c++;
			continue;
		}
		if (!n_archive || c == n_archive) {
			h = ((struct cpio *) l->data)->headers;
			while (h) {
				offset = write_cpio(h->data, offset, output);
				h      = h->next;
			}
		}
		l = l->next;
		c++;
	}

	write_trailer(offset, output);

	free_cpios(res.cpios);
	free_streams(res.streams);

	munmap(addr, (size_t) st.st_size);

	fclose(output);

	return EXIT_SUCCESS;
}
