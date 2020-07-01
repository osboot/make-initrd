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
#include "config.h"

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
print_help(void)
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
	       program_invocation_short_name);
	exit(EXIT_SUCCESS);
}

static void __attribute__((noreturn))
print_version(void)
{
	printf("%s version " PACKAGE_VERSION "\n"
	       "Written by Alexey Gladkov <gladkov.alexey@gmail.com>\n"
	       "\n"
	       "Copyright (C) 2017  Alexey Gladkov <gladkov.alexey@gmail.com>\n"
	       "This is free software; see the source for copying conditions.  There is NO\n"
	       "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
	       "\n",
	       program_invocation_short_name);
	exit(EXIT_SUCCESS);
}

static void
my_error_print_progname(void)
{
	fprintf(stderr, "%s: ", program_invocation_short_name);
}

static void __attribute__((noreturn))
bad_option_value(const char *optname, const char *value)
{
	error(EXIT_FAILURE, 0, "invalid value for \"%s\" option: %s", optname, value);
	exit(EXIT_FAILURE);
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

	error_print_progname = my_error_print_progname;

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
					error(EXIT_FAILURE, errno, "ERROR: %s: %d: fopen", __FILE__, __LINE__);
				break;
			case 'V':
				print_version();
			case 'h':
				print_help();
			default:
				exit(EXIT_FAILURE);
		}
	}

	if (optind >= argc)
		error(EXIT_FAILURE, 0, "Missing initrd file");

	if (!output)
		output = stdout;

	if (stat(argv[optind], &st) == -1)
		error(EXIT_FAILURE, errno, "ERROR: %s: %d: stat", __FILE__, __LINE__);

	if ((fd = open(argv[optind], O_RDONLY)) == -1)
		error(EXIT_FAILURE, errno, "ERROR: %s: %d: open", __FILE__, __LINE__);

	unsigned char *addr = mmap(NULL, (size_t) st.st_size, PROT_READ, MAP_PRIVATE | MAP_POPULATE, fd, 0);

	if (addr == MAP_FAILED)
		error(EXIT_FAILURE, errno, "ERROR: %s: %d: mmap", __FILE__, __LINE__);

	struct stream *s;
	struct list_tail *l, *h;
	struct result res;

	res.streams = NULL;
	res.cpios   = NULL;

	l = list_append(&res.streams, sizeof(struct stream));
	if (l == NULL)
		error(EXIT_FAILURE, errno, "unable to add element to list");
	s = l->data;

	s->addr      = addr;
	s->size      = (unsigned long) st.st_size;
	s->allocated = 0;

	read_stream("raw", s, &res);

	offset = 0;
	c      = 1;
	l      = res.cpios;
	while (l) {
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
