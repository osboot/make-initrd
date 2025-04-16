// SPDX-License-Identifier: GPL-3.0-or-later

#include "config.h"

#include <stdio.h>
#include <err.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>

int
main (int ac, char *av[])
{
	if (ac < 3)
		fprintf (stderr, "usage: %s program [arguments]\n", basename(av[0]));
	else {
		const char *path = av[2];

		av[2] = av[1];
		execvp (path, av + 2);
		err(EXIT_FAILURE, "%s", path);
	}
	return EXIT_FAILURE;
}
