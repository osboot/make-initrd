// SPDX-License-Identifier: GPL-3.0-or-later

#include "config.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compiler.h"
#include "runtime.h"

struct runtime_applet {
	const char *name;
	int (*main)(int argc, char **argv);
};

static const struct runtime_applet applets[] = {
	{ "chrooted",       chrooted_main       },
	{ "environ",        environ_main        },
	{ "halt",           halt_main           },
	{ "nfsmount",       nfsmount_main       },
	{ "poweroff",       halt_main           },
	{ "reboot",         halt_main           },
	{ "resume",         resume_main         },
	{ "runas",          runas_main          },
	{ "sysexec",        sysexec_main        },
	{ "timestamp",      timestamp_main      },
	{ "ueventd",        ueventd_main        },
	{ "ueventd-mounts", ueventd_mounts_main },
};

static const struct runtime_applet *find_applet(const char *name)
{
	size_t i;

	for (i = 0; i < sizeof(applets) / sizeof(applets[0]); i++) {
		if (!strcmp(applets[i].name, name))
			return &applets[i];
	}

	return NULL;
}

static char *base_name(char *path)
{
	char *s = strrchr(path, '/');
	return s ? s + 1 : path;
}

static void _noreturn_ usage(FILE *out, int rc)
{
	size_t i;

	fprintf(out, "Usage: runtime <applet> [args...]\n\nApplets:\n");
	for (i = 0; i < sizeof(applets) / sizeof(applets[0]); i++)
		fprintf(out, "  %s\n", applets[i].name);

	exit(rc);
}

int main(int argc, char **argv)
{
	const struct runtime_applet *applet;
	char *name;

	if (argc < 1 || !argv[0])
		usage(stderr, EXIT_FAILURE);

	name = base_name(argv[0]);
	if (name[0] == '-')
		name++;

	if (!strcmp(name, "runtime")) {
		if (argc < 2)
			usage(stderr, EXIT_FAILURE);

		if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help"))
			usage(stdout, EXIT_SUCCESS);

		argc--;
		argv++;
		name = base_name(argv[0]);
	}

	applet = find_applet(name);
	if (!applet) {
		fprintf(stderr, "runtime: unknown applet: %s\n", name);
		return 127;
	}

	program_invocation_short_name = name;
	argv[0] = name;

	return applet->main(argc, argv);
}
