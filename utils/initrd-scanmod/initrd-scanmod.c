#define _GNU_SOURCE
#include <sys/utsname.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <ctype.h>
#include <error.h>
#include <errno.h>

#include "initrd-scanmod.h"
#include "config.h"

const char short_opts[]         = "Vhb:k:";
const struct option long_opts[] = {
	{ "set-version", required_argument, 0, 'k' },
	{ "base-dir", required_argument, 0, 'b' },
	{ "help", no_argument, NULL, 'h' },
	{ "version", no_argument, NULL, 'V' },
	{ NULL, 0, NULL, 0 }
};

static void __attribute__((noreturn))
usage(int code)
{
	dprintf(STDOUT_FILENO,
	        "Usage: %s [options] [--] rules-file [rules-file ...]\n"
	        "\n"
	        "Options:\n"
	        " -k, --set-version=VERSION   use VERSION instead of `uname -r`;\n"
	        " -b, --base-dir=DIR          use DIR as filesystem root for /lib/modules;\n"
	        " -h, --help                  display this help and exit;\n"
	        " -V, --version               output version information and exit.\n"
	        "\n"
	        "Report bugs to authors.\n"
	        "\n",
	        program_invocation_short_name);
	exit(code);
}

static inline void __attribute__((noreturn))
print_version_and_exit(void)
{
	dprintf(STDOUT_FILENO, "%s version %s\n", program_invocation_short_name, PACKAGE_VERSION);
	dprintf(STDOUT_FILENO,
	        "Written by Alexey Gladkov.\n\n"
	        "Copyright (C) 2018  Alexey Gladkov <gladkov.alexey@gmail.com>\n"
	        "This is free software; see the source for copying conditions.  There is NO\n"
	        "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n");
	exit(EXIT_SUCCESS);
}

static void
my_error_print_progname(void)
{
	fprintf(stderr, "%s: ", program_invocation_short_name);
}

int
main(int argc, char **argv)
{
	int c;
	struct utsname u;
	char *kerneldir      = NULL;
	const char *kversion = NULL;
	const char *basedir  = NULL;

	error_print_progname = my_error_print_progname;

	while ((c = getopt_long(argc, argv, short_opts, long_opts, NULL)) != EOF) {
		switch (c) {
			case 'k':
				kversion = optarg;
				break;
			case 'b':
				basedir = optarg;
				break;
			case 'h':
				usage(EXIT_SUCCESS);
				break;
			case 'V':
				print_version_and_exit();
				break;
			case '?':
				usage(EXIT_FAILURE);
		}
	}

	if (argc == optind) {
		error(0, 0, "rules file required");
		usage(EXIT_FAILURE);
	}

	if (!basedir)
		basedir = "";

	if (!kversion) {
		if (uname(&u) < 0)
			error(EXIT_FAILURE, errno, "ERROR: uname()");
		kversion = u.release;
	}

	xasprintf(&kerneldir, "%s/lib/modules/%s", basedir, kversion);

	parse_rules(argc - optind, argv + optind);
	find_modules(kerneldir);

	free_rules();
	xfree(kerneldir);

	return EXIT_SUCCESS;
}
