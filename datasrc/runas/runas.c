#include <stdio.h>
#include <errno.h>
#include <error.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

extern const char *__progname;

int
main (int ac, char *av[])
{
	if (ac < 3)
		fprintf (stderr, "usage: %s program [arguments]\n", __progname);
	else
	{
		const char *path = av[2];

		av[2] = av[1];
		execvp (path, av + 2);
		error (EXIT_FAILURE, errno, "%s", path);
	}
	return EXIT_FAILURE;
}
