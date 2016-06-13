#define _GNU_SOURCE
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

#include "initrd-common.h"
#include "initrd-cpio.h"
#include "initrd-decompress.h"
#include "initrd-parse.h"
#include "initrd-ls.h"

enum flags {
	SHOW_COMPRESSION = (1 << 1),
	SHOW_NAME_ONLY   = (1 << 2),
};

static int opts = 0;

static const char cmdopts_s[] = "nCVh";
static const struct option cmdopts[] = {
	{"name",        no_argument, 0, 'n'},
	{"compression", no_argument, 0, 'C'},
	{"version",     no_argument, 0, 'V'},
	{"help",        no_argument, 0, 'h'},
	{NULL, 0, 0, 0}
};

static void __attribute__ ((noreturn))
print_help(const char *progname)
{
	printf("Usage: %s [options] initramfs\n"
		"\n"
		"Options:\n"
		"   -n, --name          Show only filenames;\n"
		"   -C, --compression   Show compression method for each archive;\n"
		"   -V, --version       Show version of program and exit;\n"
		"   -h, --help          Show this text and exit.\n"
		"\n",
		progname);
	exit(EXIT_SUCCESS);
}

static void __attribute__ ((noreturn))
print_version(const char *progname)
{
	printf("%s version " VERSION "\n"
	       "Written by Alexey Gladkov <gladkov.alexey@gmail.com>\n"
	       "\n"
	       "Copyright (C) 2016  Alexey Gladkov <gladkov.alexey@gmail.com>\n"
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
		error(EXIT_FAILURE, 0, "ERROR: Missing initrd file");

	if (stat(argv[optind], &st) == -1)
		error(EXIT_FAILURE, errno, "ERROR: %s: %d: stat", __FILE__, __LINE__);

	if ((fd = open(argv[optind], O_RDONLY)) == -1)
		error(EXIT_FAILURE, errno, "ERROR: %s: %d: open", __FILE__, __LINE__);

	unsigned char *addr = mmap(NULL, (size_t) st.st_size, PROT_READ, MAP_PRIVATE|MAP_POPULATE, fd, 0);

	if (addr == MAP_FAILED)
		error(EXIT_FAILURE, errno, "ERROR: %s: %d: mmap", __FILE__, __LINE__);

	unsigned long n_cpio = 0;
	struct stream *s;
	struct list_tail *l, *h;
	struct result res;

	res.streams = NULL;
	res.cpios = NULL;

	l = list_append(&res.streams, sizeof(struct stream));
	if (l == NULL)
		error(EXIT_FAILURE, errno, "unable to add element to list");
	s = l->data;

	s->addr = addr;
	s->size = (unsigned long) st.st_size;
	s->allocated = 0;

	read_stream("raw", s, &res);

	int bytes;
	int max_compress_name = 3;
	char *fmt = NULL;

	l = res.cpios;
	while (l) {
		if (opts & SHOW_COMPRESSION) {
			bytes = snprintf(NULL, 0, "%s", ((struct cpio *) l->data)->compress);
			if (bytes > max_compress_name)
				max_compress_name = bytes;
		}
		h = ((struct cpio *) l->data)->headers;
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
		error(EXIT_FAILURE, errno, "ERROR: %s: %d: asprintf", __FILE__, __LINE__);

	c = 1;
	l = res.cpios;
	while (l) {
		h = ((struct cpio *) l->data)->headers;
		while (h) {
			(opts & SHOW_COMPRESSION)
				? fprintf(stdout, fmt, c, ((struct cpio *) l->data)->compress)
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
