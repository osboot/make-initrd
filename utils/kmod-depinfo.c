#define _GNU_SOURCE
#include <sys/param.h>
#include <sys/utsname.h>

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <error.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include <libkmod.h>

enum show_flags {
	SHOW_DEPS     = (1 << 1),
	SHOW_MODULES  = (1 << 2),
	SHOW_FIRMWARE = (1 << 3),
	SHOW_PREFIX   = (1 << 4),
	SHOW_BUILTIN  = (1 << 5),
};

static int show_tree = 0;
static int opts      = SHOW_DEPS | SHOW_MODULES | SHOW_FIRMWARE | SHOW_PREFIX | SHOW_BUILTIN;

static char *firmware_dir;
static char firmware_defaultdir[] = "/lib/firmware/updates:/lib/firmware";

static char **modules   = NULL;
static size_t n_modules = 0;

static int
tracked_module(struct kmod_module *mod)
{
	const char *path = kmod_module_get_path(mod);

	if (modules) {
		int i   = 0;
		char *m = modules[i++];

		while (m) {
			if (!strcmp(m, path))
				return 1;
			m = modules[i++];
		}
	}

	modules = realloc(modules, (n_modules + 2) * sizeof(void *));

	if (!modules)
		error(EXIT_FAILURE, errno, "realloc: allocating %lu bytes", (n_modules + 2) * sizeof(void *));

	modules[n_modules]     = strdup(path);
	modules[n_modules + 1] = NULL;

	if (!(modules[n_modules]))
		error(EXIT_FAILURE, errno, "strdup");

	n_modules++;
	return 0;
}

static void
free_modules(void)
{
	if (!modules)
		return;

	int i   = 0;
	char *m = modules[i++];

	while (m) {
		free(m);
		m = modules[i++];
	}

	free(modules);
}

static void
process_firmware(const char *firmware)
{
	char *firmware_buf;
	char *s, *str, *token, *saveptr = NULL;
	s = str = strdup(firmware_dir);

	while ((token = strtok_r(str, ":", &saveptr)) != NULL) {
		asprintf(&firmware_buf, "%s/%s", token, firmware);

		if (!access(firmware_buf, F_OK)) {
			int i = show_tree;

			if (--i > 0) {
				while (i--)
					printf("   ");
				printf("\\_ ");
			}

			if (opts & SHOW_PREFIX)
				printf("firmware ");
			printf("%s\n", firmware_buf);
		}

		free(firmware_buf);
		str = NULL;
	}
	free(s);
}

static void
depinfo_alias(struct kmod_ctx *ctx, const char *alias);

static void
process_depends(struct kmod_ctx *ctx, const char *depends)
{
	char *s, *str, *token, *saveptr = NULL;
	s = str = strdup(depends);

	while ((token = strtok_r(str, ",", &saveptr)) != NULL) {
		depinfo_alias(ctx, token);
		str = NULL;
	}
	free(s);
}

static void
process_soft_depends(struct kmod_ctx *ctx, const char *depends)
{
	char *s, *str, *token, *saveptr = NULL;
	size_t len = strlen(depends);
	s = str = strdup(depends);

	if (len > 5 && !strncmp("pre: ", s, 5))
		str += 5;
	else if (len > 6 && !strncmp("post: ", s, 6))
		str += 6;

	while ((token = strtok_r(str, " ", &saveptr)) != NULL) {
		depinfo_alias(ctx, token);
		str = NULL;
	}
	free(s);
}

static void
depinfo(struct kmod_ctx *ctx, struct kmod_module *mod)
{
	struct kmod_list *l, *list = NULL;
	int ret;

	if (tracked_module(mod))
		return;

	if (opts & SHOW_MODULES) {
		int i = show_tree;

		if (--i > 0) {
			while (i--)
				printf("   ");
			printf("\\_ ");
		}

		if (opts & SHOW_PREFIX)
			printf("module ");

		printf("%s\n", kmod_module_get_path(mod));

		if (show_tree)
			show_tree++;
	}

	if ((ret = kmod_module_get_info(mod, &list)) < 0)
		error(EXIT_FAILURE, ret, "ERROR: Could not get information from '%s'",
		      kmod_module_get_name(mod));

	kmod_list_foreach(l, list)
	{
		const char *key = kmod_module_info_get_key(l);
		const char *val = kmod_module_info_get_value(l);

		if ((opts & SHOW_DEPS) && (opts & SHOW_MODULES)) {
			if (!strcmp("depends", key))
				process_depends(ctx, val);
			else if (!strcmp("softdep", key))
				process_soft_depends(ctx, val);

		} else if ((opts & SHOW_FIRMWARE) && !strcmp("firmware", key))
			process_firmware(val);
	}

	kmod_module_info_free_list(list);

	if (show_tree > 1)
		show_tree--;
}

static void
depinfo_path(struct kmod_ctx *ctx, const char *path)
{
	struct kmod_module *mod;

	if (kmod_module_new_from_path(ctx, path, &mod) < 0)
		error(EXIT_FAILURE, 0, "ERROR: Module file %s not found.", path);

	depinfo(ctx, mod);
	kmod_module_unref(mod);
}

static void
depinfo_alias(struct kmod_ctx *ctx, const char *alias)
{
	struct kmod_module *mod;
	struct kmod_list *l, *filtered, *list = NULL;

	if (kmod_module_new_from_lookup(ctx, alias, &list) < 0)
		error(EXIT_FAILURE, 0, "ERROR: Module alias %s not found.", alias);

	if (!list)
		error(EXIT_FAILURE, 0, "ERROR: Module %s not found.", alias);

	if (kmod_module_apply_filter(ctx, KMOD_FILTER_BUILTIN, list, &filtered) < 0)
		error(EXIT_FAILURE, 0, "ERROR: Failed to filter list: %m");

	if (!filtered) {
		if (opts & SHOW_BUILTIN) {
			kmod_list_foreach(l, list)
			{
				mod = kmod_module_get_module(l);

				if (opts & SHOW_PREFIX)
					printf("builtin ");

				printf("%s\n", kmod_module_get_name(mod));

				kmod_module_unref(mod);
			}
		}

		kmod_module_unref_list(list);
		return;
	}

	kmod_list_foreach(l, filtered)
	{
		mod = kmod_module_get_module(l);
		depinfo(ctx, mod);
		kmod_module_unref(mod);
	}

	kmod_module_unref_list(filtered);
	kmod_module_unref_list(list);
}

static const char cmdopts_s[]        = "k:b:f:tDMFPBVh";
static const struct option cmdopts[] = {
	{ "tree", no_argument, 0, 't' },
	{ "no-deps", no_argument, 0, 'D' },
	{ "no-modules", no_argument, 0, 'M' },
	{ "no-firmware", no_argument, 0, 'F' },
	{ "no-prefix", no_argument, 0, 'P' },
	{ "no-builtin", no_argument, 0, 'B' },
	{ "set-version", required_argument, 0, 'k' },
	{ "base-dir", required_argument, 0, 'b' },
	{ "firmware-dir", required_argument, 0, 'f' },
	{ "version", no_argument, 0, 'V' },
	{ "help", no_argument, 0, 'h' },
	{ NULL, 0, 0, 0 }
};

static void __attribute__((noreturn))
print_help(const char *progname)
{
	printf("Usage: %s [options] [module|filename|alias]...\n"
	       "\n"
	       "Displays the full path to module and its dependencies.\n"
	       "It also shows the full path to firmware.\n"
	       "\n"
	       "Options:\n"
	       "   -t, --tree                  Show dependencies between modules hierarchically;\n"
	       "   -D, --no-deps               Don't show dependence;\n"
	       "   -P, --no-prefix             Omit the prefix describing the type of file;\n"
	       "   -M, --no-modules            Omit modules from the output;\n"
	       "   -F, --no-firmware           Omit firmware from the output;\n"
	       "   -B, --no-builtin            Omit builtin modules from the output;\n"
	       "   -k, --set-version=VERSION   Use VERSION instead of `uname -r`;\n"
	       "   -b, --base-dir=DIR          Use DIR as filesystem root for /lib/modules;\n"
	       "   -f, --firmware-dir=DIR      Use DIR as colon-separated list of firmware directories\n"
	       "                               (default: %s);\n"
	       "   -V, --version               Show version of program and exit;\n"
	       "   -h, --help                  Show this text and exit.\n"
	       "\n",
	       progname, firmware_defaultdir);
	exit(EXIT_SUCCESS);
}

static void __attribute__((noreturn))
print_version(const char *progname)
{
	printf("%s version " VERSION "\n"
	       "Written by Alexey Gladkov <gladkov.alexey@gmail.com>\n"
	       "\n"
	       "Copyright (C) 2012  Alexey Gladkov <gladkov.alexey@gmail.com>\n"
	       "This is free software; see the source for copying conditions.  There is NO\n"
	       "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
	       "\n",
	       progname);
	exit(EXIT_SUCCESS);
}

int
main(int argc, char **argv)
{
	struct kmod_ctx *ctx;
	struct utsname u;
	char *module_dir;
	const char *kversion = NULL;
	const char *base_dir = NULL;
	int i, c;

	while ((c = getopt_long(argc, argv, cmdopts_s, cmdopts, NULL)) != -1) {
		switch (c) {
			case 'D':
				opts ^= SHOW_DEPS;
				break;
			case 'M':
				opts ^= SHOW_MODULES;
				break;
			case 'F':
				opts ^= SHOW_FIRMWARE;
				break;
			case 'P':
				opts ^= SHOW_PREFIX;
				break;
			case 'B':
				opts ^= SHOW_BUILTIN;
				break;
			case 't':
				show_tree = 1;
				break;
			case 'k':
				kversion = optarg;
				break;
			case 'b':
				base_dir = optarg;
				break;
			case 'f':
				firmware_dir = optarg;
				break;
			case 'V':
				print_version(basename(argv[0]));
			case 'h':
				print_help(basename(argv[0]));
		}
	}

	if (optind >= argc)
		error(EXIT_FAILURE, 0, "ERROR: Missing module or filename.");

	if (!(opts & SHOW_MODULES) && !(opts & SHOW_FIRMWARE))
		error(EXIT_FAILURE, 0, "ERROR: Options --no-modules and --no-firmware are mutually exclusive.");

	if (!firmware_dir)
		firmware_dir = firmware_defaultdir;

	if (!base_dir)
		base_dir = "";

	if (!kversion) {
		if (uname(&u) < 0)
			error(EXIT_FAILURE, errno, "ERROR: uname()");
		kversion = u.release;
	}

	asprintf(&module_dir, "%s/lib/modules/%s", base_dir, kversion);

	if (!(ctx = kmod_new(module_dir, NULL)))
		error(EXIT_FAILURE, errno, "ERROR: kmod_new()");

	free(module_dir);

	for (i = optind; i < argc; i++) {
		access(argv[i], F_OK)
		    ? depinfo_alias(ctx, argv[i])
		    : depinfo_path(ctx, argv[i]);
	}

	kmod_unref(ctx);
	free_modules();

	return EXIT_SUCCESS;
}
