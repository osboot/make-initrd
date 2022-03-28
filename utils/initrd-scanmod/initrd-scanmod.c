#define _GNU_SOURCE
#include <sys/utsname.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <ctype.h>
#include <err.h>

#include "initrd-scanmod.h"
#include "config.h"

int verbose = 0;

const char short_opts[]         = "Vhb:k:v";
const struct option long_opts[] = {
	{ "set-version", required_argument, 0, 'k' },
	{ "base-dir", required_argument, 0, 'b' },
	{ "help", no_argument, NULL, 'h' },
	{ "verbose", no_argument, NULL, 'v' },
	{ "version", no_argument, NULL, 'V' },
	{ NULL, 0, NULL, 0 }
};

static void __attribute__((noreturn))
usage(const char *progname, int code)
{
	dprintf(STDOUT_FILENO,
	        "Usage: %s [options] [--] rules-file [rules-file ...]\n"
	        "\n"
	        "Options:\n"
	        " -k, --set-version=VERSION   use VERSION instead of `uname -r`;\n"
	        " -b, --base-dir=DIR          use DIR as filesystem root for /lib/modules;\n"
	        " -v, --verbose               print a message for each action;\n"
	        " -h, --help                  display this help and exit;\n"
	        " -V, --version               output version information and exit.\n"
	        "\n"
	        "Report bugs to authors.\n"
	        "\n",
	        progname);
	exit(code);
}

static inline void __attribute__((noreturn))
print_version_and_exit(const char *progname)
{
	dprintf(STDOUT_FILENO, "%s version %s\n", progname, PACKAGE_VERSION);
	dprintf(STDOUT_FILENO,
	        "Written by Alexey Gladkov.\n\n"
	        "Copyright (C) 2018  Alexey Gladkov <gladkov.alexey@gmail.com>\n"
	        "This is free software; see the source for copying conditions.  There is NO\n"
	        "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n");
	exit(EXIT_SUCCESS);
}

int
main(int argc, char **argv)
{
	int c;
	struct utsname u;
	char *kerneldir      = NULL;
	const char *kversion = NULL;
	const char *basedir  = NULL;

	while ((c = getopt_long(argc, argv, short_opts, long_opts, NULL)) != EOF) {
		switch (c) {
			case 'k':
				kversion = optarg;
				break;
			case 'b':
				basedir = optarg;
				break;
			case 'v':
				verbose++;
				break;
			case 'h':
				usage(basename(argv[0]), EXIT_SUCCESS);
				break;
			case 'V':
				print_version_and_exit(basename(argv[0]));
				break;
			case '?':
				usage(basename(argv[0]), EXIT_FAILURE);
		}
	}

	if (argc == optind) {
		warnx("rules file required");
		usage(basename(argv[0]), EXIT_FAILURE);
	}

	if (!basedir)
		basedir = "";

	if (!kversion) {
		if (uname(&u) < 0)
			err(EXIT_FAILURE, "ERROR: uname()");
		kversion = u.release;
	}

	xasprintf(&kerneldir, "%s/lib/modules/%s", basedir, kversion);

	if (verbose)
		warnx("kernel directory: %s", kerneldir);

	parse_rules(argc - optind, argv + optind);
	find_modules(kerneldir);

	free_rules();
	xfree(kerneldir);

	return EXIT_SUCCESS;
}
