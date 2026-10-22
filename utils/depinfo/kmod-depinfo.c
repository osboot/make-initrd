// SPDX-License-Identifier: GPL-3.0-or-later

#include "config.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <sys/utsname.h>
#include <sys/queue.h>

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <glob.h>
#include <fnmatch.h>
#include <libgen.h>
#include <libkmod.h>
#include <err.h>

#include "temp_failure_retry.h"

enum alias_need {
	ALIAS_OPTIONAL = 0,
	ALIAS_REQUIRED = 1,
};

enum show_flags {
	SHOW_DEPS             = (1 << 1),
	SHOW_MODULES          = (1 << 2),
	SHOW_FIRMWARE         = (1 << 3),
	SHOW_PREFIX           = (1 << 4),
	SHOW_BUILTIN          = (1 << 5),
	SHOW_MISSING_FIRMWARE = (1 << 6),
};

static int show_tree = 0;
static int opts      = SHOW_DEPS | SHOW_MODULES | SHOW_FIRMWARE | SHOW_PREFIX | SHOW_BUILTIN;

static const char *kversion = NULL;
static const char *suffixes[] = { "", ".zst", ".xz", ".bz2", ".gz", NULL };

#define SYS_FIRMWARE_CLASS_PATH "/sys/module/firmware_class/parameters/path"

static char *firmware_dir;
static char firmware_defaultdir[] = "/lib/firmware/updates:/lib/firmware";
static char firmware_class_path[PATH_MAX + 1] = { 0 };

static int use_blacklist = 0;

struct string_entry {
	char *str;
	STAILQ_ENTRY(string_entry) entries;
};

STAILQ_HEAD(string_head, string_entry);

static struct string_head tracked_modules = STAILQ_HEAD_INITIALIZER(tracked_modules);

struct kernel_builtin {
	char *name;
	struct string_head aliases;
	struct string_head firmware;
	int tracked;

	STAILQ_ENTRY(kernel_builtin) entries;
};

STAILQ_HEAD(kernel_builtin_head, kernel_builtin);

static struct kernel_builtin_head kbuiltin = STAILQ_HEAD_INITIALIZER(kbuiltin);

static struct kernel_builtin *
find_kernel_builtin(const char *str)
{
	struct kernel_builtin *b;
	struct string_entry *e;

	if (!str || !*str)
		return NULL;

	// First we are looking for a name match.
	STAILQ_FOREACH(b, &kbuiltin, entries) {
		if (!strcmp(b->name, str))
			return b;
	}

	// Second we are looking for a match in aliases.
	STAILQ_FOREACH(b, &kbuiltin, entries) {
		STAILQ_FOREACH(e, &b->aliases, entries) {
			if (!fnmatch(e->str, str, FNM_NOESCAPE))
				return b;
		}
	}

	return NULL;
}

static struct kernel_builtin *
append_kernel_builtin(const char *name)
{
	struct kernel_builtin *b;

	STAILQ_FOREACH(b, &kbuiltin, entries) {
		if (!strcmp(b->name, name))
			return b;
	}

	b = calloc(1, sizeof(*b));
	if (!b)
		errx(EXIT_FAILURE, "memory allocation failed");

	b->name = strdup(name);
	if (!b->name)
		errx(EXIT_FAILURE, "memory allocation failed");

	STAILQ_INIT(&b->aliases);
	STAILQ_INIT(&b->firmware);
	b->tracked = 0;

	STAILQ_INSERT_TAIL(&kbuiltin, b, entries);
	return b;
}

static void
append_string_entry(struct string_head *head, const char *str)
{
	struct string_entry *e;

	if (!str || !*str)
		return;

	e = malloc(sizeof(*e));
	if (!e)
		errx(EXIT_FAILURE, "memory allocation failed");

	e->str = strdup(str);
	if (!e->str)
		errx(EXIT_FAILURE, "memory allocation failed");

	STAILQ_INSERT_TAIL(head, e, entries);
}

static void
free_string_list(struct string_head *head)
{
	struct string_entry *e;

	while ((e = STAILQ_FIRST(head)) != NULL) {
		STAILQ_REMOVE_HEAD(head, entries);
		free(e->str);
		free(e);
	}
}

static void
free_kernel_builtin(void)
{
	struct kernel_builtin *b;

	while ((b = STAILQ_FIRST(&kbuiltin)) != NULL) {
		STAILQ_REMOVE_HEAD(&kbuiltin, entries);
		free(b->name);
		free_string_list(&b->aliases);
		free_string_list(&b->firmware);
		free(b);
	}
}

static int
read_kernel_builtin(char *module_dir, const char *modinfo)
{
	int fd;
	char filename[MAXPATHLEN];
	struct stat st;

	snprintf(filename, MAXPATHLEN - 1, "%s/%s", module_dir, modinfo);

	errno = 0;
	if ((fd = open(filename, O_RDONLY)) < 0) {
		if (errno == ENOENT)
			return 0;
		warn("open: %s", filename);
		return -1;
	}

	if (fstat(fd, &st) < 0) {
		warn("fstat: %s", filename);
		close(fd);
		return -1;
	}

	char *addr, *p, *s;

	addr = mmap(NULL, (size_t) st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (addr == MAP_FAILED) {
		warn("mmap: %s", filename);
		close(fd);
		return -1;
	}

	size_t bytes = 0;
	p = addr;

	while (bytes < (size_t) st.st_size) {
		s = strdup(p);
		if (!s)
			errx(EXIT_FAILURE, "memory allocation failed");

		char *value = strchr(s, '=');
		if (!value) {
			warnx("ERROR: Unexpected file format: %s (ignored).", filename);
			free(s);
			goto ignore;
		}
		*value = '\0';
		value++;

		char *field = strrchr(s, '.');
		if (!field) {
			warnx("ERROR: Unexpected file format: %s (ignored).", filename);
			free(s);
			goto ignore;
		}
		*field = '\0';
		field++;

		struct kernel_builtin *b = append_kernel_builtin(s);

		if (!strcmp(field, "alias"))
			append_string_entry(&b->aliases, value);
		else if (!strcmp(field, "firmware"))
			append_string_entry(&b->firmware, value);

		free(s);

		bytes += strlen(p) + 1;
		p = addr + bytes;
	}

	munmap(addr, (size_t) st.st_size);
	close(fd);

	return 0;
ignore:
	munmap(addr, (size_t) st.st_size);
	close(fd);

	free_kernel_builtin();

	return 0;
}

static int
tracked_module(struct kmod_module *mod)
{
	struct string_entry *e;
	const char *path = kmod_module_get_path(mod);

	if (!path)
		return 0;

	STAILQ_FOREACH(e, &tracked_modules, entries) {
		if (!strcmp(e->str, path))
			return 1;
	}

	append_string_entry(&tracked_modules, path);
	return 0;
}

static void
free_modules(void)
{
	free_string_list(&tracked_modules);
}

static void
show_with_prefix(int depth, const char *keyword, const char *value)
{
	if (--depth > 0) {
		while (depth--)
			printf("   ");
		printf("\\_ ");
	}

	if (opts & SHOW_PREFIX)
		printf("%s ", keyword);

	printf("%s\n", value);
}

static int
expand_pattern(const char *keyword, const char *pattern)
{
	size_t i;
	glob_t globbuf;

	if (!access(pattern, F_OK)) {
		show_with_prefix(show_tree, keyword, pattern);
		return 1;
	}

	glob(pattern, 0, NULL, &globbuf);

	for (i = 0; i < globbuf.gl_pathc; i++)
		show_with_prefix(show_tree, keyword, globbuf.gl_pathv[i]);

	globfree(&globbuf);

	return globbuf.gl_pathc > 0;
}

static int
process_firmware_pattern(const char *basedir, size_t baselen, const char *pattern)
{
	char path[MAXPATHLEN];
	int n, found = 0;

	if (baselen >= sizeof(path)) {
		warnx("firmware path is too long: %s", basedir);
		return -1;
	}

	strncpy(path, basedir, baselen);

	for (n = 0; suffixes[n]; n++) {
		snprintf(path + baselen, sizeof(path) - baselen, "/%s/%s%s", kversion, pattern, suffixes[n]);
		if ((found = expand_pattern("firmware", path)))
			break;

		snprintf(path + baselen, sizeof(path) - baselen, "/%s%s", pattern, suffixes[n]);
		if ((found = expand_pattern("firmware", path)))
			break;
	}

	if (opts & SHOW_MISSING_FIRMWARE && !found)
		show_with_prefix(show_tree, "missing-firmware", pattern);

	return found;
}

static void
process_firmware(const char *pattern)
{
	const char *begin = firmware_dir;

	if (*firmware_class_path &&
	    process_firmware_pattern(firmware_class_path, strlen(firmware_class_path), pattern))
		return;

	while (*begin) {
		const char *end = strchr(begin, ':');
		size_t len = end ? (size_t) (end - begin) : strlen(begin);

		if (len > 0 && process_firmware_pattern(begin, len, pattern))
			return;

		if (!end)
			break;

		begin = end + 1;
	}
}

static int
process_builtin(const char *name)
{
	struct kernel_builtin *b = find_kernel_builtin(name);
	struct string_entry *e;

	if (b) {
		if (b->tracked)
			return 0;
		b->tracked = 1;
	}

	if (opts & SHOW_BUILTIN) {
		show_with_prefix(show_tree, "builtin", b ? b->name : name);
		if (show_tree)
			show_tree++;
	}

	if ((opts & SHOW_FIRMWARE) && b) {
		STAILQ_FOREACH(e, &b->firmware, entries)
			process_firmware(e->str);
	}

	if ((opts & SHOW_BUILTIN) && show_tree > 1)
		show_tree--;

	return 0;
}

static int
module_in_list(struct kmod_module *mod, struct kmod_list *list)
{
	struct kmod_list *l;
	const char *name = kmod_module_get_name(mod);

	if (!list)
		return 0;

	kmod_list_foreach(l, list) {
		struct kmod_module *m = kmod_module_get_module(l);
		int match = !strcmp(kmod_module_get_name(m), name);
		kmod_module_unref(m);
		if (match)
			return 1;
	}
	return 0;
}

static int
depinfo(struct kmod_ctx *ctx, struct kmod_module *mod);

static int
process_module_list(struct kmod_ctx *ctx, struct kmod_list *list)
{
	int ret = -1;
	struct kmod_module *mod;
	struct kmod_list *l;
	struct kmod_list *filtered = NULL;

	if (!list)
		return 0;

	if (use_blacklist) {
		if (kmod_module_apply_filter(ctx, KMOD_FILTER_BLACKLIST, list, &filtered) < 0) {
			warn("ERROR: Failed to filter (blacklist) list");
			goto end;
		}

		if (!filtered) {
			ret = 0;
			goto end;
		}

		kmod_module_unref_list(list);
		list = filtered;
		filtered = NULL;
	}

	if (kmod_module_apply_filter(ctx, KMOD_FILTER_BUILTIN, list, &filtered) < 0) {
		warn("ERROR: Failed to filter list");
		goto end;
	}

	ret = 0;

	kmod_list_foreach(l, list) {
		mod = kmod_module_get_module(l);
		if (!module_in_list(mod, filtered))
			process_builtin(kmod_module_get_name(mod));
		kmod_module_unref(mod);
	}

	if (filtered) {
		kmod_list_foreach(l, filtered) {
			mod = kmod_module_get_module(l);
			if (find_kernel_builtin(kmod_module_get_name(mod))) {
				process_builtin(kmod_module_get_name(mod));
				kmod_module_unref(mod);
				continue;
			}
			if (depinfo(ctx, mod) < 0)
				ret = -1;
			kmod_module_unref(mod);
		}
	}
end:
	if (filtered)
		kmod_module_unref_list(filtered);
	if (list)
		kmod_module_unref_list(list);

	return ret;
}

static int
depinfo_alias(struct kmod_ctx *ctx, const char *alias, enum alias_need req)
{
	struct kmod_list *list = NULL;

	if (find_kernel_builtin(alias))
		return process_builtin(alias);

	if (kmod_module_new_from_lookup(ctx, alias, &list) < 0) {
		if (req == ALIAS_OPTIONAL)
			return 0;
		warnx("ERROR: Module alias %s not found.", alias);
		return -1;
	}

	if (!list) {
		if (req == ALIAS_OPTIONAL)
			return 0;
		warnx("ERROR: Module %s not found.", alias);
		return -1;
	}

	return process_module_list(ctx, list);
}

static int
process_soft_deps(struct kmod_ctx *ctx, struct kmod_module *mod)
{
	struct kmod_list *pre = NULL, *post = NULL;
	int ret = 0;

	if (kmod_module_get_softdeps(mod, &pre, &post) < 0)
		return 0;

	if (pre && process_module_list(ctx, pre) < 0)
		ret = -1;

	if (post && process_module_list(ctx, post) < 0)
		ret = -1;

	return ret;
}

static int
process_weak_deps(struct kmod_ctx *ctx, struct kmod_module *mod)
{
#ifdef HAVE_KMOD_MODULE_GET_WEAKDEPS
	struct kmod_list *weak = NULL;

	if (kmod_module_get_weakdeps(mod, &weak) < 0)
		return 0;

	if (weak && process_module_list(ctx, weak) < 0)
		return -1;
#endif
	return 0;
}

static int
__process_depends(struct kmod_ctx *ctx, const char *depends, const char *delim, enum alias_need req)
{
	int ret = 0;
	char *s, *str, *token, *saveptr = NULL;

	if (!depends || !*depends)
		return 0;

	s = str = strdup(depends);
	if (!s)
		errx(EXIT_FAILURE, "memory allocation failed");

	while ((token = strtok_r(str, delim, &saveptr)) != NULL) {
		if (depinfo_alias(ctx, token, req) < 0)
			ret = -1;
		str = NULL;
	}
	free(s);
	return ret;
}

static int
process_depends(struct kmod_ctx *ctx, const char *depends)
{
	return __process_depends(ctx, depends, ", \t\r\n", ALIAS_REQUIRED);
}

static int
process_soft_depends(struct kmod_ctx *ctx, const char *depends)
{
	int ret = 0;
	char *s, *str, *token, *saveptr = NULL;

	if (!depends || !*depends)
		return 0;

	s = str = strdup(depends);
	if (!s)
		errx(EXIT_FAILURE, "memory allocation failed");

	while ((token = strtok_r(str, " \t\r\n", &saveptr)) != NULL) {
		str = NULL;

		if (!strcmp(token, "pre:") || !strcmp(token, "post:"))
			continue;

		if (!strncmp(token, "pre:", 4))
			token += 4;
		else if (!strncmp(token, "post:", 5))
			token += 5;

		if (!*token)
			continue;

		if (depinfo_alias(ctx, token, ALIAS_OPTIONAL) < 0)
			ret = -1;
	}

	free(s);
	return ret;
}

static int
process_weak_depends(struct kmod_ctx *ctx, const char *depends)
{
	return __process_depends(ctx, depends, " \t\r\n", ALIAS_OPTIONAL);
}

static int
depinfo(struct kmod_ctx *ctx, struct kmod_module *mod)
{
	struct kmod_list *l, *list = NULL;
	int ret;

	ret = tracked_module(mod);

	switch (ret) {
		case  1:
			return  0;
		case -1:
			return -1;
	}

	if ((ret = kmod_module_get_info(mod, &list)) < 0) {
		errno = ret;
		warn("ERROR: Could not get information from '%s'",
		     kmod_module_get_name(mod));
		return -1;
	}

	if (opts & SHOW_MODULES) {
		show_with_prefix(show_tree, "module", kmod_module_get_path(mod));

		if (show_tree)
			show_tree++;
	}

	kmod_list_foreach(l, list) {
		const char *key = kmod_module_info_get_key(l);
		const char *val = kmod_module_info_get_value(l);

		if ((opts & SHOW_DEPS) && (opts & SHOW_MODULES)) {
			if (!strcmp("depends", key)) {
				if (process_depends(ctx, val) < 0)
					ret = -1;
			} else if (!strcmp("softdep", key)) {
				if (process_soft_depends(ctx, val) < 0)
					ret = -1;
			} else if (!strcmp("weakdep", key)) {
				if (process_weak_depends(ctx, val) < 0)
					ret = -1;
			}
		}
		if ((opts & SHOW_FIRMWARE) && !strcmp("firmware", key))
			process_firmware(val);
	}

	kmod_module_info_free_list(list);

	if ((opts & SHOW_DEPS) && (opts & SHOW_MODULES)) {
		if (process_soft_deps(ctx, mod) < 0)
			ret = -1;
		if (process_weak_deps(ctx, mod) < 0)
			ret = -1;
	}

	if (show_tree > 1)
		show_tree--;

	return ret;
}

static int
depinfo_path(struct kmod_ctx *ctx, const char *path)
{
	int ret = -1;
	struct kmod_module *mod;

	if (kmod_module_new_from_path(ctx, path, &mod) < 0) {
		warnx("ERROR: Module file %s not found.", path);
		return ret;
	}

	ret = depinfo(ctx, mod);
	kmod_module_unref(mod);

	return ret;
}

static int
is_filename(const char *path)
{
	struct stat sb;

	if (strlen(path) < 2 || (path[1] != '/' && path[0] != '.' && path[0] != '/'))
		return 1;

	if (!strchr(path, '.'))
		return 1;

	if (stat(path, &sb) < 0 || !S_ISREG(sb.st_mode))
		return 1;

	return 0;
}

static inline int
process_name(struct kmod_ctx *ctx, const char *arg)
{
	return is_filename(arg)
	       ? depinfo_alias(ctx, arg, ALIAS_REQUIRED)
	       : depinfo_path(ctx, arg);
}

static int
read_names(struct kmod_ctx *ctx, const char *file)
{
	FILE *stream;

	if (!file)
		return 0;

	if (!strcmp(file, "-"))
		stream = stdin;
	else
		stream = fopen(file, "r");

	if (!stream) {
		warn("fopen: %s", file);
		return -1;
	}

	int ret = 0;
	char *line = NULL;
	size_t len = 0;
	ssize_t nread;

	while ((nread = getline(&line, &len, stream)) != -1) {
		char *arg, *end;

		arg = line;
		end = arg + (nread - 1);

		for (; arg < end; ++arg) {
			if (!isspace(*arg))
				break;
		}
		for (; end >= arg; --end) {
			if (!isspace(*end))
				break;
			*end = '\0';
		}

		if (!strlen(arg))
			continue;

		if (process_name(ctx, arg) < 0)
			ret = -1;
	}

	free(line);

	if (stream != stdin)
		fclose(stream);

	return ret;
}


static const char cmdopts_s[]        = "k:b:f:i:tDMFPBVh";
static const struct option cmdopts[] = {
	{ "use-blacklist", no_argument, 0, 1, },
	{ "missing-firmware", no_argument, 0, 2 },
	{ "tree", no_argument, 0, 't' },
	{ "no-deps", no_argument, 0, 'D' },
	{ "no-modules", no_argument, 0, 'M' },
	{ "no-firmware", no_argument, 0, 'F' },
	{ "no-prefix", no_argument, 0, 'P' },
	{ "no-builtin", no_argument, 0, 'B' },
	{ "set-version", required_argument, 0, 'k' },
	{ "base-dir", required_argument, 0, 'b' },
	{ "firmware-dir", required_argument, 0, 'f' },
	{ "input", required_argument, 0, 'i' },
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
	       "       --use-blacklist         Apply blacklist commands in the configuration files;\n"
	       "       --missing-firmware      Show firmware that could not be found;\n"
	       "   -i, --input=FILE            Read names from FILE;\n"
	       "   -V, --version               Show version of program and exit;\n"
	       "   -h, --help                  Show this text and exit.\n"
	       "\n",
	       progname, firmware_defaultdir);
	exit(EXIT_SUCCESS);
}

static void __attribute__((noreturn))
print_version(const char *progname)
{
	printf("%s version " PACKAGE_VERSION "\n"
	       "Written by Alexey Gladkov <gladkov.alexey@gmail.com>\n"
	       "\n"
	       "Copyright (C) 2012-2024  Alexey Gladkov <gladkov.alexey@gmail.com>\n"
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
	char module_dir[MAXPATHLEN];
	const char *base_dir = NULL;
	const char *from_file = NULL;
	int i, c;

	while ((c = getopt_long(argc, argv, cmdopts_s, cmdopts, NULL)) != -1) {
		switch (c) {
			case 1:
				use_blacklist = 1;
				break;
			case 2:
				opts |= SHOW_MISSING_FIRMWARE;
				break;
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
			case 'i':
				from_file = optarg;
				break;
			case 'V':
				print_version(basename(argv[0]));
			case 'h':
				print_help(basename(argv[0]));
		}
	}

	if (!from_file && optind >= argc)
		errx(EXIT_FAILURE, "ERROR: Missing module or filename.");

	if (!firmware_dir)
		firmware_dir = firmware_defaultdir;

	if (!base_dir)
		base_dir = "";

	if (!kversion) {
		if (uname(&u) < 0)
			err(EXIT_FAILURE, "ERROR: uname()");
		kversion = u.release;
	}

	if (!access(SYS_FIRMWARE_CLASS_PATH, F_OK)) {
		int fd = open(SYS_FIRMWARE_CLASS_PATH, O_RDONLY | O_CLOEXEC);
		if (fd != -1) {
			ssize_t len = TEMP_FAILURE_RETRY(read(fd, firmware_class_path, PATH_MAX));
			close(fd);

			if (len > 0) {
				firmware_class_path[len] = '\0';

				while (len > 0 && firmware_class_path[len - 1] == '\n')
					firmware_class_path[--len] = '\0';
			}
		}
	}

	snprintf(module_dir, sizeof(module_dir), "%s/lib/modules/%s", base_dir, kversion);

	if (read_kernel_builtin(module_dir, "kernel.builtin.modinfo") < 0)
		err(EXIT_FAILURE, "ERROR: read_kernel_builtin()");

	if (read_kernel_builtin(module_dir, "modules.builtin.modinfo") < 0)
		err(EXIT_FAILURE, "ERROR: read_kernel_builtin()");

	if (!(ctx = kmod_new(module_dir, NULL)))
		err(EXIT_FAILURE, "ERROR: kmod_new()");

	int ret = EXIT_SUCCESS;

	if (read_names(ctx, from_file) < 0)
		ret = EXIT_FAILURE;

	for (i = optind; i < argc; i++) {
		if (process_name(ctx, argv[i]) < 0)
			ret = EXIT_FAILURE;
	}

	kmod_unref(ctx);
	free_modules();
	free_kernel_builtin();

	return ret;
}
