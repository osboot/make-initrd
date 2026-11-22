// SPDX-License-Identifier: GPL-3.0-or-later

#include "config.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <err.h>

#include "compiler.h"
#include "temp_failure_retry.h"
#include "initrd-common.h"
#include "initrd-cpio.h"

static void show_help(int rc) __noreturn;
static void show_version(void) __noreturn;
static char *read_stream(int fd, size_t *out_len);
static char *read_file(const char *path, size_t *out_len, struct stat *st_out);
static const char *sanitize_path(const char *path);
static unsigned long get_stream_offset(FILE *fp);
static void write_entry(const char *arc_name, const char *body, size_t body_len,
                        mode_t mode, time_t mtime, uid_t uid, gid_t gid, FILE *out);
static void write_archive_trailer(FILE *out);
static int cmd_add(int argc, char **argv);
static int cmd_trailer(int argc, char **argv);
static int cmd_create(int argc, char **argv);

static void __noreturn
show_help(int rc)
{
	printf("Usage: cpio-stream <command> [options]\n"
	       "\n"
	       "Commands:\n"
	       "   add [options] <archive-path> [<file>]  Add an entry to the archive\n"
	       "   finish [options]                      Write archive trailer\n"
	       "   trailer [options]                     Alias for finish\n"
	       "   create [options] <file>...            Create archive from files\n"
	       "\n"
	       "Options for 'add':\n"
	       "   -m, --mode=MODE     File permissions mode (octal, default: 0644)\n"
	       "   -t, --mtime=TIME    Modification timestamp (default: current time)\n"
	       "   -u, --uid=UID       User ID (default: 0)\n"
	       "   -g, --gid=GID       Group ID (default: 0)\n"
	       "   -o, --output=FILE   Write output to FILE instead of stdout\n"
	       "\n"
	       "Options for 'finish' / 'trailer' / 'create':\n"
	       "   -o, --output=FILE   Write output to FILE instead of stdout\n"
	       "\n"
	       "General options:\n"
	       "   -V, --version       Show version and exit\n"
	       "   -h, --help          Show this text and exit\n"
	       "\n");
	exit(rc);
}

static void __noreturn
show_version(void)
{
	printf("cpio-stream version " PACKAGE_VERSION "\n"
	       "Written by Alexey Gladkov <gladkov.alexey@gmail.com>\n"
	       "\n"
	       "Copyright (C) 2026  Alexey Gladkov <gladkov.alexey@gmail.com>\n"
	       "This is free software; see the source for copying conditions.  There is NO\n"
	       "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
	       "\n");
	exit(EXIT_SUCCESS);
}

static char *
read_stream(int fd, size_t *out_len)
{
	size_t cap = 65536;
	size_t len = 0;
	char *buf = malloc(cap);

	if (!buf)
		err(EXIT_FAILURE, "malloc failed");

	while (1) {
		if (len == cap) {
			size_t new_cap = cap * 2;
			char *new_buf;

			if (new_cap < cap) {
				free(buf);
				errx(EXIT_FAILURE, "file too large");
			}
			new_buf = realloc(buf, new_cap);
			if (!new_buf) {
				free(buf);
				err(EXIT_FAILURE, "realloc failed");
			}
			buf = new_buf;
			cap = new_cap;
		}

		ssize_t n = TEMP_FAILURE_RETRY(read(fd, buf + len, cap - len));
		if (n < 0) {
			free(buf);
			err(EXIT_FAILURE, "read failed");
		}
		if (n == 0)
			break;
		len += (size_t) n;
	}

	*out_len = len;
	return buf;
}

static char *
read_file(const char *path, size_t *out_len, struct stat *st_out)
{
	struct stat st;
	int fd;
	char *buf;
	size_t len = 0;
	size_t fsize;

	if (stat(path, &st) < 0)
		err(EXIT_FAILURE, "stat failed: %s", path);

	if (st_out)
		*st_out = st;

	if (!S_ISREG(st.st_mode)) {
		fd = open(path, O_RDONLY);
		if (fd < 0)
			err(EXIT_FAILURE, "open failed: %s", path);
		buf = read_stream(fd, out_len);
		close(fd);
		return buf;
	}

	fsize = (size_t) st.st_size;
	buf = malloc(fsize > 0 ? fsize : 1);
	if (!buf)
		err(EXIT_FAILURE, "malloc failed");

	fd = open(path, O_RDONLY);
	if (fd < 0) {
		free(buf);
		err(EXIT_FAILURE, "open failed: %s", path);
	}

	while (len < fsize) {
		ssize_t n = TEMP_FAILURE_RETRY(read(fd, buf + len, fsize - len));
		if (n < 0) {
			free(buf);
			close(fd);
			err(EXIT_FAILURE, "read failed: %s", path);
		}
		if (n == 0)
			break;
		len += (size_t) n;
	}
	close(fd);

	*out_len = len;
	return buf;
}

static const char *
sanitize_path(const char *path)
{
	while (*path == '/')
		path++;
	while (strncmp(path, "./", 2) == 0)
		path += 2;
	if (*path == '\0')
		errx(EXIT_FAILURE, "invalid archive path");
	return path;
}

static unsigned long
get_stream_offset(FILE *fp)
{
	off_t cur = lseek(fileno(fp), 0, SEEK_CUR);
	return (cur >= 0) ? (unsigned long) cur : 0;
}

static void
write_entry(const char *arc_name, const char *body, size_t body_len,
            mode_t mode, time_t mtime, uid_t uid, gid_t gid, FILE *out)
{
	struct cpio_header hdr;
	unsigned long offset;

	memset(&hdr, 0, sizeof(hdr));
	hdr.ino      = 1;
	hdr.mode     = (mode_t) ((mode & 07777) | S_IFREG);
	hdr.uid      = uid;
	hdr.gid      = gid;
	hdr.nlink    = 1;
	hdr.mtime    = mtime ? mtime : time(NULL);
	hdr.body_len = (unsigned long) body_len;
	hdr.name_len = (unsigned long) (strlen(arc_name) + 1);
	hdr.name     = (char *) (uintptr_t) arc_name;
	hdr.body     = (char *) (uintptr_t) body;

	offset = get_stream_offset(out);
	write_cpio(&hdr, offset, out);

	if (fflush(out) != 0 || ferror(out))
		err(EXIT_FAILURE, "write error");
}

static void
write_archive_trailer(FILE *out)
{
	unsigned long offset = get_stream_offset(out);
	write_trailer(offset, out);

	if (fflush(out) != 0 || ferror(out))
		err(EXIT_FAILURE, "write error");
}

static int
cmd_add(int argc, char **argv)
{
	static const struct option long_options[] = {
		{ "mode",    required_argument, NULL, 'm' },
		{ "mtime",   required_argument, NULL, 't' },
		{ "uid",     required_argument, NULL, 'u' },
		{ "gid",     required_argument, NULL, 'g' },
		{ "output",  required_argument, NULL, 'o' },
		{ "help",    no_argument,       NULL, 'h' },
		{ NULL, 0, NULL, 0 }
	};

	mode_t mode = 0;
	time_t mtime = 0;
	uid_t uid = 0;
	gid_t gid = 0;
	int has_mode = 0, has_mtime = 0, has_uid = 0, has_gid = 0;
	const char *outfile = NULL;
	const char *arc_name;
	const char *src_file;
	char *body = NULL;
	size_t body_len = 0;
	struct stat st;
	int from_file;
	FILE *out;
	int c;

	optind = 1;
	while ((c = getopt_long(argc, argv, "m:t:u:g:o:h", long_options, NULL)) != -1) {
		char *endptr = NULL;
		switch (c) {
			case 'm':
				mode = (mode_t) strtoul(optarg, &endptr, 8);
				if (!optarg[0] || *endptr)
					errx(EXIT_FAILURE, "invalid mode: %s", optarg);
				has_mode = 1;
				break;
			case 't':
				mtime = (time_t) strtoul(optarg, &endptr, 10);
				if (!optarg[0] || *endptr)
					errx(EXIT_FAILURE, "invalid mtime: %s", optarg);
				has_mtime = 1;
				break;
			case 'u':
				uid = (uid_t) strtoul(optarg, &endptr, 10);
				if (!optarg[0] || *endptr)
					errx(EXIT_FAILURE, "invalid uid: %s", optarg);
				has_uid = 1;
				break;
			case 'g':
				gid = (gid_t) strtoul(optarg, &endptr, 10);
				if (!optarg[0] || *endptr)
					errx(EXIT_FAILURE, "invalid gid: %s", optarg);
				has_gid = 1;
				break;
			case 'o':
				outfile = optarg;
				break;
			case 'h':
				show_help(EXIT_SUCCESS);
				break;
			default:
				show_help(EXIT_FAILURE);
		}
	}

	if (optind >= argc)
		errx(EXIT_FAILURE, "add: missing archive file name");

	arc_name = sanitize_path(argv[optind]);
	src_file = (optind + 1 < argc) ? argv[optind + 1] : NULL;
	from_file = (src_file && strcmp(src_file, "-") != 0);

	if (from_file) {
		body = read_file(src_file, &body_len, &st);
		if (!has_mode)
			mode = st.st_mode;
		if (!has_mtime)
			mtime = st.st_mtime;
		if (!has_uid)
			uid = st.st_uid;
		if (!has_gid)
			gid = st.st_gid;
	} else {
		body = read_stream(STDIN_FILENO, &body_len);
		if (!has_mode)
			mode = 0644;
		if (!has_mtime)
			mtime = time(NULL);
	}

	out = stdout;
	if (outfile) {
		out = fopen(outfile, "ab");
		if (!out) {
			free(body);
			err(EXIT_FAILURE, "cannot open output file: %s", outfile);
		}
	}

	write_entry(arc_name, body, body_len, mode, mtime, uid, gid, out);

	if (outfile)
		fclose(out);
	free(body);

	return EXIT_SUCCESS;
}

static int
cmd_trailer(int argc, char **argv)
{
	static const struct option long_options[] = {
		{ "output",  required_argument, NULL, 'o' },
		{ "help",    no_argument,       NULL, 'h' },
		{ NULL, 0, NULL, 0 }
	};

	const char *outfile = NULL;
	FILE *out;
	int c;

	optind = 1;
	while ((c = getopt_long(argc, argv, "o:h", long_options, NULL)) != -1) {
		switch (c) {
			case 'o':
				outfile = optarg;
				break;
			case 'h':
				show_help(EXIT_SUCCESS);
				break;
			default:
				show_help(EXIT_FAILURE);
		}
	}

	out = stdout;
	if (outfile) {
		out = fopen(outfile, "ab");
		if (!out)
			err(EXIT_FAILURE, "cannot open output file: %s", outfile);
	}

	write_archive_trailer(out);

	if (outfile)
		fclose(out);

	return EXIT_SUCCESS;
}

static int
cmd_create(int argc, char **argv)
{
	static const struct option long_options[] = {
		{ "output",  required_argument, NULL, 'o' },
		{ "help",    no_argument,       NULL, 'h' },
		{ NULL, 0, NULL, 0 }
	};

	const char *outfile = NULL;
	FILE *out;
	int c;
	int i;

	optind = 1;
	while ((c = getopt_long(argc, argv, "o:h", long_options, NULL)) != -1) {
		switch (c) {
			case 'o':
				outfile = optarg;
				break;
			case 'h':
				show_help(EXIT_SUCCESS);
				break;
			default:
				show_help(EXIT_FAILURE);
		}
	}

	if (optind >= argc)
		errx(EXIT_FAILURE, "create: no input files specified");

	out = stdout;
	if (outfile) {
		out = fopen(outfile, "wb");
		if (!out)
			err(EXIT_FAILURE, "cannot open output file: %s", outfile);
	}

	for (i = optind; i < argc; i++) {
		const char *path = argv[i];
		const char *arc_name = sanitize_path(path);
		size_t body_len = 0;
		struct stat st;
		char *body = read_file(path, &body_len, &st);

		write_entry(arc_name, body, body_len, st.st_mode, st.st_mtime,
		            st.st_uid, st.st_gid, out);
		free(body);
	}

	write_archive_trailer(out);

	if (outfile)
		fclose(out);

	return EXIT_SUCCESS;
}

int
main(int argc, char **argv)
{
	int cmd_idx = -1;
	const char *cmd = NULL;
	char **sub_argv;
	int sub_argc;
	int ret = EXIT_FAILURE;
	int i;

	if (argc < 2)
		show_help(EXIT_FAILURE);

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0 ||
		    strcmp(argv[i], "help") == 0)
			show_help(EXIT_SUCCESS);

		if (strcmp(argv[i], "-V") == 0 || strcmp(argv[i], "--version") == 0 ||
		    strcmp(argv[i], "version") == 0)
			show_version();

		if (argv[i][0] != '-' && cmd_idx < 0) {
			if (strcmp(argv[i], "add") == 0 ||
			    strcmp(argv[i], "finish") == 0 ||
			    strcmp(argv[i], "trailer") == 0 ||
			    strcmp(argv[i], "create") == 0) {
				cmd_idx = i;
				cmd = argv[i];
			}
		}
	}

	if (!cmd)
		errx(EXIT_FAILURE, "no command specified; run 'cpio-stream --help' for usage");

	sub_argv = malloc((size_t) argc * sizeof(char *));
	if (!sub_argv)
		err(EXIT_FAILURE, "malloc failed");

	sub_argv[0] = argv[cmd_idx];
	sub_argc = 1;
	for (i = 1; i < argc; i++) {
		if (i == cmd_idx)
			continue;
		sub_argv[sub_argc++] = argv[i];
	}
	sub_argv[sub_argc] = NULL;

	if (strcmp(cmd, "add") == 0)
		ret = cmd_add(sub_argc, sub_argv);
	else if (strcmp(cmd, "finish") == 0 || strcmp(cmd, "trailer") == 0)
		ret = cmd_trailer(sub_argc, sub_argv);
	else if (strcmp(cmd, "create") == 0)
		ret = cmd_create(sub_argc, sub_argv);

	free(sub_argv);
	return ret;
}
