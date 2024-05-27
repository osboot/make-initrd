// SPDX-License-Identifier: GPL-3.0-or-later
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>    /* strcmp */
#include <stdio.h>
#include <limits.h>
#include <err.h>       /* err */

static char *prog;

static void show_usage(int rc)
{
	fprintf(stderr, "Usage: %s -h\n", prog);
	fprintf(stderr, "   or: %s <rootdir> test -{b|c|d|e|f|p|s|L} <path>\n", prog);
	fprintf(stderr, "   or: %s <rootdir> realpath <path>\n", prog);
	exit(rc);
}

static int cmd_test(int argc, char **argv)
{
	struct stat st;

	if (argc != 3)
		errx(EXIT_FAILURE, "wrong number of arguments");

	if (argv[1][0] != '-')
		errx(EXIT_FAILURE, "unexpected option: `%s'", argv[1]);

	if (argv[1][1] == 'L') {
		if (lstat(argv[2], &st) < 0)
			return 1;
	} else {
		if (stat(argv[2], &st) < 0)
			return 1;
	}

	switch(argv[1][1]) {
		case 'b':
			return !S_ISBLK(st.st_mode);
		case 'c':
			return !S_ISCHR(st.st_mode);
		case 'd':
			return !S_ISDIR(st.st_mode);
		case 'e':
			return 0;
		case 'f':
			return !S_ISREG(st.st_mode);
		case 'p':
			return !S_ISFIFO(st.st_mode);
		case 's':
			return (!S_ISREG(st.st_mode) && st.st_size > 0);
		case 'L':
			return !S_ISLNK(st.st_mode);
	}

	return 0;
}

static int cmd_realpath(int argc, char **argv)
{
	char path[PATH_MAX+1];

	if (argc != 2)
		errx(EXIT_FAILURE, "wrong number of arguments");

	if (!realpath(argv[1], path))
		err(EXIT_FAILURE, "realpath: %s", argv[1]);

	puts(path);

	return 0;
}

int main(int argc, char **argv)
{
	prog = strrchr(argv[0], '/');
	if (!prog)
		prog = argv[0];
	else
		prog++;

	if (argc < 3)
		show_usage(EXIT_FAILURE);
	else if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help"))
		show_usage(EXIT_SUCCESS);

	if (chroot(argv[1]) < 0)
		err(EXIT_FAILURE, "chroot: %s", argv[1]);

	if (chdir("/") < 0)
		err(EXIT_FAILURE, "chdir");

	if (!strcmp(argv[2], "test"))
		return cmd_test(argc - 2, argv + 2);

	if (!strcmp(argv[2], "realpath"))
		return cmd_realpath(argc - 2, argv + 2);

	warnx("unknown command: %s", argv[2]);
	return EXIT_FAILURE;
}
