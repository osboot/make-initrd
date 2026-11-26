/*
 * Handle resume from suspend-to-disk
 */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>

#include "compiler.h"
#include "resume.h"
#include "runtime.h"

static char *progname;

static void _noreturn_ usage(void)
{
	fprintf(stderr, "Usage: %s /dev/<resumedevice> [offset]\n", progname);
	exit(1);
}

int resume_main(int argc, char *argv[])
{
	progname = argv[0];
	if (argc < 2 || argc > 3)
		usage();

	return resume(argv[1], (argc > 2) ? strtoull(argv[2], NULL, 0) : 0ULL);
}
