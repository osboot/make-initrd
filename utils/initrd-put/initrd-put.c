// SPDX-License-Identifier: GPL-3.0-or-later

#include "config.h"

#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/sendfile.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <getopt.h>
#include <errno.h>
#include <err.h>
#include <sysexits.h>
#include <fts.h>
#include <fcntl.h>
#include <ctype.h>
#include <regex.h>
#include <string.h>

#include "temp_failure_retry.h"
#include "memory.h"
#include "queue.h"
#include "tree.h"
#include "enqueue.h"

static const char *progname = NULL;

static char *destdir = NULL;
static size_t destdir_len = 0;
char *prefix = NULL;
static size_t prefix_len = 0;
static char *logfile = NULL;
int verbose = 0;
static int dry_run = 0;
static int force = 0;
static size_t installed = 0;

regex_t *exclude_match = NULL;
size_t exclude_match_nr = 0;

static void show_version(void) __attribute__((__noreturn__));
static void show_help(int rc) __attribute__((__noreturn__));

static void fill_stat(struct file *p, struct stat *sb) __attribute__((__nonnull__ (1, 2)));
static void enqueue_parent_directory(char *path);
static void enqueue_directory(char *path) __attribute__((__nonnull__ (1)));
static int mksock(const char *path) __attribute__((__nonnull__ (1)));

static bool suffix_requires_dir_check(char const *end) __attribute__((__nonnull__ (1)));
static bool dir_check(const char *dir, char *dirend) __attribute__((__nonnull__ (1, 2)));
static void enqueue_canonicalized_path(const char *name, bool add_recursively);

static int enqueue_regular_file(const char *filename) __attribute__((__nonnull__ (1)));
static void enqueue_path(struct file *p) __attribute__((__nonnull__ (1)));
static void print_file(struct file *p) __attribute__((__nonnull__ (1)));
static void install_file(struct file *p) __attribute__((__nonnull__ (1)));
static void apply_permissions(struct file *p) __attribute__((__nonnull__ (1)));

void fill_stat(struct file *p, struct stat *sb)
{
	memcpy(&p->stat, sb, sizeof(p->stat));
}

void enqueue_parent_directory(char *path)
{
	if (!path || (prefix && !strcmp(path, prefix)))
		return;

	char *p = strrchr(path, '/');
	if (!p || p == path)
		return;

	enqueue_item(path, p - path);
}

void enqueue_directory(char *path)
{
	FTS *t = NULL;
	char *argv[2] = { path, NULL };

	if (verbose)
		warnx("processing: %s", argv[0]);

	if ((t = fts_open(argv, FTS_PHYSICAL, NULL)) == NULL)
		err(EXIT_FAILURE, "fts_open");

	FTSENT *p;
	while ((p = fts_read(t))) {
		switch (p->fts_info) {
			case FTS_DNR:
			case FTS_ERR:
			case FTS_NS:
				errno = p->fts_errno;
				warn("fts_read: %s", p->fts_path);
				continue;
		}

		if (is_path_added(p->fts_path))
			continue;

		struct file *f = enqueue_item(p->fts_path, -1);
		if (f)
			fill_stat(f, p->fts_statp);
	}

	fts_close(t);
}

int mksock(const char *path)
{
	struct sockaddr_un sun = { 0 };
	int ret = -1;

	errno = EINVAL;
	if (strlen(path) < sizeof(sun)) {
		int fd = -1;

		sun.sun_family = AF_UNIX;
		strcpy(sun.sun_path, path);

		if ((fd = socket(PF_UNIX, SOCK_STREAM, 0)) >= 0 &&
		    !bind(fd, (struct sockaddr *) &sun, sizeof(sun))) {
			errno = 0;
			ret = 0;
		}
		if (fd >= 0)
			close(fd);
	}

	return ret;
}

static char const dir_suffix[] = "/./";

#define IS_DIR_SEPARATOR(c) ((c) == '/')
#define IS_ABSOLUTE_FILE_NAME(f) IS_DIR_SEPARATOR((f)[0])

/*
 * True if concatenating END as a suffix to a file name means that the
 * code needs to check that the file name is that of a searchable
 * directory, since the canonicalize_filename_mode_stk code won't
 * check this later anyway when it checks an ordinary file name
 * component within END.  END must either be empty, or start with a
 * slash.
 */
bool suffix_requires_dir_check(char const *end)
{
	/* If END does not start with a slash, the suffix is OK. */
	while (IS_DIR_SEPARATOR(*end)) {
		/* Two or more slashes act like a single slash. */
		do {
			end++;
		} while (IS_DIR_SEPARATOR(*end));

		switch (*end++) {
			default:
				/* An ordinary file name component is OK. */
				return false;
			case '\0':
				/* Trailing "/" is trouble. */
				return true;
			case '.':
				/* Possibly "." or "..". */
				break;
		}
		/* Trailing "/.", or "/.." even if not trailing, is trouble. */
		if (!*end || (*end == '.' && (!end[1] || IS_DIR_SEPARATOR(end[1]))))
			return true;
	}
	return false;
}

/*
 * Return true if DIR is a searchable dir, false (setting errno) otherwise.
 * DIREND points to the NUL byte at the end of the DIR string.
 * Store garbage into DIREND[0 .. strlen (dir_suffix)].
 */
bool dir_check(const char *dir, char *dirend)
{
	strcpy(dirend, dir_suffix);

	/*
	 * Return true if FILE's existence can be shown, false (setting errno)
	 * otherwise.  Follow symbolic links.
	 */
	struct stat st;
	return stat(dir, &st) == 0 || errno == EOVERFLOW;
}

void enqueue_canonicalized_path(const char *name, bool add_recursively)
{
	char rname[PATH_MAX + 1];
	char link_buffer[PATH_MAX + 1];
	char extra_buffer[PATH_MAX + 1];
	int num_links = 0;
	char *dest;
	const char *start;
	const char *end = NULL;
	bool end_in_extra_buffer = false;
	bool failed = true;

	if (name == NULL) {
		errno = EINVAL;
		err(EXIT_FAILURE, "enqueue_canonicalized_path: parameter is a null pointer");
	}

	if (name[0] == '\0') {
		errno = ENOENT;
		err(EXIT_FAILURE, "enqueue_canonicalized_path: parameter points to an empty string");
	}

	if (!IS_ABSOLUTE_FILE_NAME(name)) {
		if (getcwd(rname, sizeof(rname)) == NULL)
			err(EXIT_FAILURE, "unable to get current directory");
		dest = rname + strlen(rname);
	} else {
		dest = rname;
		*dest++ = '/';
	}

	for (start = name; *start; start = end) {
		/*
		 * Skip sequence of multiple file name separators.
		 */
		while (IS_DIR_SEPARATOR(*start))
			start++;

		/*
		 * Find end of component.
		 */
		for (end = start; *end && !IS_DIR_SEPARATOR(*end); ++end);

		/*
		 * Length of this file name component; it can be zero if a file
		 * name ends in '/'.
		 */
		size_t startlen = (size_t)(end - start);

		if (startlen == 0) {
			break;
		} else if (startlen == 1 && start[0] == '.') {
			/* Nothing */;
		} else if (startlen == 2 && start[0] == '.' && start[1] == '.') {
			if (dest > rname + 1) {
				for (--dest; dest > rname && !IS_DIR_SEPARATOR(dest[-1]); --dest)
					continue;
			}
		} else {
			if (!IS_DIR_SEPARATOR(dest[-1]))
				*dest++ = '/';

			dest = mempcpy(dest, start, startlen);
			*dest = '\0';

			char *buf = link_buffer;
			ssize_t n;

			errno = 0;
			n = readlink(rname, buf, sizeof(link_buffer) - 1);

			if (0 <= n) {
				if (++num_links > MAXSYMLINKS) {
					errno = ELOOP;
					goto error;
				}

				buf[n] = '\0';

				if (verbose > 1)
					warnx("symlink '%s' points to '%s'", rname, buf);

				enqueue_item(rname, -1);

				char *extra_buf = extra_buffer;
				size_t end_idx = 0;
				size_t len = strlen(end);

				if (end_in_extra_buffer)
					end_idx = (size_t)(end - extra_buf);

				if (sizeof(extra_buffer) <= len + (size_t) n) {
					errno = ENAMETOOLONG;
					err(EXIT_FAILURE, "bad path '%s'", name);
				}

				if (end_in_extra_buffer)
					end = extra_buf + end_idx;

				/* Careful here, end may be a pointer into extra_buf... */
				memmove(&extra_buf[n], end, len + 1);
				name = end = memcpy(extra_buf, buf, (size_t) n);
				end_in_extra_buffer = true;

				if (IS_ABSOLUTE_FILE_NAME(buf)) {
					dest = rname;
					*dest++ = '/'; /* It's an absolute symlink */
				} else {
					if (dest > rname + 1) {
						for (--dest; dest > rname && !IS_DIR_SEPARATOR(dest[-1]); --dest)
							continue;
					}
				}
			} else if (!(suffix_requires_dir_check(end)
			             ? dir_check(rname, dest)
			             : errno == EINVAL)) {
				goto error;
			}
		}
	}
	if (dest > rname + 1 && IS_DIR_SEPARATOR(dest[-1]))
		dest--;
	failed = false;

error:
	*dest++ = '\0';

	if (failed)
		err(EXIT_FAILURE, "unable to process component of path: %s", rname);

	errno = 0;

	if (verbose > 0)
		warnx("symlink '%s' points to '%s'", name, rname);

	struct file *f;

	f = enqueue_item(rname, dest - rname);
	if (f)
		f->recursive = add_recursively;
}

int enqueue_regular_file(const char *filename)
{
	static char buf[LINE_MAX];
	int fd, ret = -1;

	errno = 0;
	fd = open(filename, O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
	if (fd < 0) {
		warn("open: %s", filename);
		if (errno == EACCES || errno == EPERM)
			return 0;
		return -1;
	}

	buf[0] = 0;
	if (pread(fd, buf, sizeof(buf), 0) < 0) {
		warn("read: %s", filename);
		goto end;
	}

	buf[LINE_MAX - 1] = 0;

	if (!access(filename, X_OK) && is_shebang(buf)) {
		ret = enqueue_shebang(filename, buf);
		if (ret)
			goto end;
	}

	if (is_elf_file(buf)) {
		ret = enqueue_libraries(filename, fd);
		if (ret)
			goto end;
	}

	ret = 0;
end:
	close(fd);
	return ret;
}

void enqueue_path(struct file *p)
{
	if (!is_path_added(p->src))
		return;

	if (!p->stat.st_ino) {
		struct stat sb;

		if (lstat(p->src, &sb) < 0)
			err(EXIT_FAILURE, "lstat: %s", p->src);

		fill_stat(p, &sb);
	}

	enqueue_parent_directory(p->src);

	if (S_IFDIR == (p->stat.st_mode & S_IFMT)) {
		if (p->recursive)
			enqueue_directory(p->src);
		return;
	}

	if (S_IFLNK == (p->stat.st_mode & S_IFMT)) {
		char symlink[PATH_MAX + 1];
		ssize_t linklen = readlink(p->src, symlink, sizeof(symlink));

		if (linklen >= 0) {
			symlink[linklen] = 0;
			p->symlink = xstrdup(symlink);
		} else {
			warn("readlink: %s", p->src);
		}

		enqueue_canonicalized_path(p->src, false);
		return;
	}

	if (S_IFREG == (p->stat.st_mode & S_IFMT)) {
		if (enqueue_regular_file(p->src) < 0)
			err(EXIT_FAILURE, "failed to read regular file: %s", p->src);
		return;
	}
}

static FILE *logout;

void print_file(struct file *p)
{
	char type;

	switch (p->stat.st_mode & S_IFMT) {
		case S_IFBLK:
			type = 'b';
			break;
		case S_IFCHR:
			type = 'c';
			break;
		case S_IFDIR:
			type = 'd';
			break;
		case S_IFIFO:
			type = 'p';
			break;
		case S_IFLNK:
			type = 'l';
			break;
		case S_IFREG:
			type = 'f';
			break;
		case S_IFSOCK:
			type = 's';
			break;
		default:
			type = '?';
			break;
	}

	fprintf(logout, "%c\t%s\t%s%s\t%s\n",
	        type,
	        p->src,
	        destdir, p->dst,
	        (p->symlink ? p->symlink : ""));
}

static char install_path[PATH_MAX + 1];
static int use_copy_file_range = 1;
static int use_sendfile = 1;

void install_file(struct file *p)
{
	const char *ftype;
	const char *op = "install";

	if (p->installed)
		return;

	switch (p->stat.st_mode & S_IFMT) {
		case S_IFBLK:
			ftype = "block device";
			break;
		case S_IFCHR:
			ftype = "character device";
			break;
		case S_IFDIR:
			ftype = "directory";
			break;
		case S_IFIFO:
			ftype = "FIFO/pipe";
			break;
		case S_IFLNK:
			ftype = "symlink";
			break;
		case S_IFREG:
			ftype = "regular file";
			break;
		case S_IFSOCK:
			ftype = "socket";
			break;
		default:
			ftype = "unknown";
			break;
	}

	strncpy(install_path + destdir_len, p->dst, sizeof(install_path) - destdir_len - 1);

	errno = 0;
	if (force && (S_IFDIR != (p->stat.st_mode & S_IFMT)) &&
	    remove(install_path) < 0 && errno != ENOENT)
		err(EXIT_FAILURE, "remove: %s", install_path);

	if (S_IFDIR == (p->stat.st_mode & S_IFMT)) {
		if (verbose > 2)
			warnx("create a directory: %s", install_path);
		errno = 0;
		if (mkdir(install_path, 0755) < 0) {
			if (errno == EEXIST) {
				op = "skip";
				goto end;
			}
			if (errno == ENOENT)
				return;
			err(EX_CANTCREAT, "mkdir: %s", install_path);
		}
		goto end;
	}

	if (S_IFBLK == (p->stat.st_mode & S_IFMT) ||
	    S_IFCHR == (p->stat.st_mode & S_IFMT)) {
		if (verbose > 2)
			warnx("make a special file: %s", install_path);
		errno = 0;
		if (mknod(install_path, p->stat.st_mode, p->stat.st_dev) < 0) {
			if (errno == EEXIST) {
				op = "skip";
				goto end;
			}
			if (errno == ENOENT)
				return;
			err(EX_CANTCREAT, "mknod: %s", install_path);
		}
		goto end;
	}

	if (S_IFLNK == (p->stat.st_mode & S_IFMT)) {
		if (verbose > 2)
			warnx("create a symlink file: %s", install_path);
		errno = 0;
		if (symlink(p->symlink, install_path) < 0) {
			if (errno == EEXIST) {
				op = "skip";
				goto end;
			}
			if (errno == ENOENT)
				return;
			err(EX_CANTCREAT, "symlink: %s", install_path);
		}
		goto end;
	}

	if (S_IFIFO == (p->stat.st_mode & S_IFMT)) {
		if (verbose > 2)
			warnx("create a fifo file: %s", install_path);
		errno = 0;
		if (mkfifo(install_path, p->stat.st_mode) < 0) {
			if (errno == EEXIST) {
				op = "skip";
				goto end;
			}
			if (errno == ENOENT)
				return;
			err(EX_CANTCREAT, "mkfifo: %s", install_path);
		}
		goto end;
	}

	if (S_IFSOCK == (p->stat.st_mode & S_IFMT)) {
		if (verbose > 2)
			warnx("create a socket file: %s", install_path);
		errno = 0;
		if (mksock(install_path) < 0) {
			if (errno == EEXIST) {
				op = "skip";
				goto end;
			}
			if (errno == ENOENT)
				return;
			err(EX_CANTCREAT, "mksock: %s", install_path);
		}
		goto end;
	}

	if (S_IFREG != (p->stat.st_mode & S_IFMT))
		errx(EXIT_FAILURE, "not implemented (mode=%o): %s", p->stat.st_mode, p->dst);

	if (!access(install_path, X_OK)) {
		op = "skip";
		goto end;
	}

	if (verbose > 2)
		warnx("create a regular file: %s", install_path);

	int sfd, dfd;

	errno = 0;
	if ((sfd = open(p->src, O_RDONLY)) < 0)
		err(EX_NOINPUT, "open: %s", p->src);

	if ((dfd = creat(install_path, p->stat.st_mode)) < 0) {
		if (errno == ENOENT) {
			close(sfd);
			return;
		}
		err(EX_CANTCREAT, "creat: %s", install_path);
	}

	posix_fadvise(sfd, 0, p->stat.st_size, POSIX_FADV_SEQUENTIAL);
	posix_fallocate(dfd, 0, p->stat.st_size);

	off_t ret;
	off_t len = p->stat.st_size;

	if (!use_copy_file_range)
		goto fallback_sendfile;
	do {
		errno = 0;
		ret = copy_file_range(sfd, NULL, dfd, NULL, (size_t) len, 0);
		if (ret < 0) {
			if (errno == EXDEV || errno == ENOSYS) {
				use_copy_file_range = 0;
				if (verbose > 2)
					warnx("copy_file_range not supported");
				goto fallback_sendfile;
			}
			err(EX_IOERR, "copy_file_range: %s -> %s", p->src, p->dst);
		}
		len -= ret;
	} while (len > 0 && ret > 0);
finish:
	close(sfd);
	close(dfd);
end:
	if (verbose)
		warnx("%s (%s): %s", op, ftype, install_path);
	p->installed = true;
	installed++;
	return;

fallback_sendfile:
	if (!use_sendfile)
		goto fallback_readwrite;

	lseek(sfd, 0, SEEK_SET);
	lseek(dfd, 0, SEEK_SET);

	len = p->stat.st_size;
	do {
		errno = 0;
		ret = sendfile(dfd, sfd, NULL, (size_t) len);
		if (ret < 0) {
			if (errno == EINVAL || errno == ENOSYS) {
				use_sendfile = 0;
				if (verbose > 2)
					warnx("sendfile not supported");
				goto fallback_readwrite;
			}
			err(EX_IOERR, "sendfile: %s -> %s", p->src, p->dst);
		}
		len -= ret;
	} while (len > 0 && ret > 0);

	goto finish;

fallback_readwrite:
	lseek(sfd, 0, SEEK_SET);
	lseek(dfd, 0, SEEK_SET);

	char buf[BUFSIZ];
	len = p->stat.st_size;
	do {
		ret = TEMP_FAILURE_RETRY(read(sfd, buf, sizeof(buf)));
		if (ret < 0)
			err(EX_IOERR, "read: %s", p->src);

		ret = TEMP_FAILURE_RETRY(write(dfd, buf, (size_t) ret));
		if (ret < 0)
			err(EX_IOERR, "write: %s", p->dst);

		len -= ret;
	} while (len > 0 && ret > 0);

	goto finish;
}

void apply_permissions(struct file *p)
{
	strncpy(install_path + destdir_len, p->dst, sizeof(install_path) - destdir_len - 1);

	errno = 0;
	if (lchown(install_path, p->stat.st_uid, p->stat.st_gid) < 0) {
		if (errno != EPERM || verbose > 2)
			warn("unable to change owner and group to uid=%d and gid=%d of `%s'", p->stat.st_uid, p->stat.st_gid, install_path);
		if (errno != EPERM)
			exit(EXIT_FAILURE);
	}

	if (S_IFLNK != (p->stat.st_mode & S_IFMT)) {
		errno = 0;
		if (chmod(install_path, p->stat.st_mode) < 0)
			err(EXIT_FAILURE, "change file mode of `%s' to %jo", install_path, (uintmax_t) p->stat.st_mode);
	}
}

void show_help(int rc)
{
	fprintf(stdout,
	        "Usage: %1$s [<options>] <destdir> directory [directory ...]\n"
	        "   or: %1$s [<options>] <destdir> file [file ...]\n"
	        "\n"
	        "Utility allows to copy files and directories along with their dependencies\n"
	        "into a specified destination directory.\n"
	        "\n"
	        "This utility follows symbolic links and binary dependencies and copies them\n"
	        "along with the specified files.\n"
	        "\n"
	        "Options:\n"
	        "   -n, --dry-run              don't do nothing.\n"
	        "   -e, --exclude=REGEXP       exclude files matching REGEXP.\n"
	        "   -f, --force                overwrite destination file if exists.\n"
	        "   -l, --log=FILE             white log about what was copied.\n"
	        "   -r, --remove-prefix=PATH   ignore prefix in path.\n"
	        "   -v, --verbose              print a message for each action/\n"
	        "   -V, --version              output version information and exit.\n"
	        "   -h, --help                 display this help and exit.\n"
	        "\n"
	        "Report bugs to authors.\n"
	        "\n",
	        progname
	       );
	exit(rc);
}

void show_version(void)
{
	fprintf(stdout,
	        "%1$s version " PACKAGE_VERSION "\n"
	        "Written by Alexey Gladkov.\n"
	        "\n"
	        "Copyright (C) 2012-2024  Alexey Gladkov <gladkov.alexey@gmail.com>\n"
	        "This is free software; see the source for copying conditions.  There is NO\n"
	        "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n",
	        progname
	       );
	exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
	const char *optstring = "efnl:r:vVh";
	const struct option longopts[] = {
		{"exclude", required_argument, 0, 'e' },
		{"remove-prefix", required_argument, 0, 'r' },
		{"force", no_argument, 0, 'f' },
		{"dry-run", no_argument, 0, 'n' },
		{"log", required_argument, 0, 'l' },
		{"verbose", no_argument, 0, 'v' },
		{"version", no_argument, 0, 'V' },
		{"help", no_argument, 0, 'h' },
		{ 0, 0, 0, 0 }
	};
	int c;

	progname = strrchr(argv[0], '/');
	if (progname)
		progname++;
	else
		progname = argv[0];

	while ((c = getopt_long(argc, argv, optstring, longopts, NULL)) != -1) {
		switch (c) {
			case 'e':
				if (strlen(optarg) > 0) {
					exclude_match = xrealloc(exclude_match, (exclude_match_nr + 1), sizeof(regex_t));

					if (regcomp(&exclude_match[exclude_match_nr], optarg, REG_NOSUB | REG_NEWLINE | REG_EXTENDED))
						errx(EX_USAGE, "bad regexp");

					exclude_match_nr++;
				}

				break;
			case 'f':
				force = 1;
				break;
			case 'l':
				logfile = optarg;
				break;
			case 'n':
				dry_run = 1;
				break;
			case 'r':
				prefix = optarg;
				prefix_len = strlen(optarg);
				break;
			case 'v':
				verbose++;
				break;
			case 'V':
				show_version();
				break;
			case 'h':
				show_help(EXIT_SUCCESS);
				break;
			default:
				fprintf(stderr, "Try '%s --help' for more information.\n",
				        progname);
				return EX_USAGE;
		}
	}

	if (optind == argc)
		errx(EX_USAGE, "more arguments required");

	if ((destdir = realpath(argv[optind], NULL)) == NULL)
		errx(EX_USAGE, "bad destination directory: %s", argv[optind]);

	destdir_len = strlen(destdir);
	optind++;

	if (optind == argc)
		errx(EX_USAGE, "more arguments required");

	init_elf_library();

	for (int i = optind; i < argc; i++) {
		enqueue_canonicalized_path(argv[i], true);
	}

	strncpy(install_path, destdir, sizeof(install_path) - 1);
	install_path[destdir_len] = 0;

	for (struct file *queue = get_queue(NULL); queue; queue = get_queue(NULL)) {
		while (queue != NULL) {
			struct file *next = queue->next;

			queue->dst = queue->src;

			if (prefix) {
				size_t dst_len = strlen(queue->dst);
				if (dst_len >= prefix_len && queue->dst[prefix_len] == '/' &&
				    !strncmp(queue->dst, prefix, prefix_len - 1))
					queue->dst += prefix_len;
				else if (!strcmp(queue->dst, prefix))
					queue->dst = (char *) "";
			}

			if (!force) {
				struct stat st;

				strncpy(install_path + destdir_len, queue->dst, sizeof(install_path) - destdir_len - 1);

				errno = 0;
				if (lstat(install_path, &st) < 0) {
					if (errno != ENOENT)
						err(EX_OSERR, "unable to get access to %s", install_path);
				} else if (!S_ISDIR(st.st_mode)) {
					if (verbose > 1)
						warnx("'%s' is already in the destdir", queue->src);
					dequeue_item(queue);
					free_file(queue);
					queue = next;
					continue;
				}
			}

			if (tree_add_file(queue)) {
				enqueue_path(queue);
				dequeue_item(queue);
			} else {
				if (verbose > 1)
					warnx("'%s' has already been processed so skip it", queue->src);
				dequeue_item(queue);
				free_file(queue);
			}

			queue = next;
		}
	}

	if (dry_run) {
		if (verbose > 1)
			warnx("dry run only ...");
		logout = stdout;
		tree_walk(print_file);
	} else {
		size_t queue_nr = 0;

		if (verbose > 1)
			warnx("copying files ...");

		umask(0);

		while (1) {
			size_t prev_installed = installed;

			tree_walk(install_file);

			if (prev_installed == installed)
				break;
		}

		get_queue(&queue_nr);

		if (queue_nr > installed) {
			logout = stdout;
			tree_walk(print_file);
			errx(EXIT_FAILURE, "unable to create the files listed above");
		}

		tree_walk(apply_permissions);
	}

	if (logfile) {
		if ((logout = fopen(logfile, "a")) == NULL)
			err(EX_CANTCREAT, "open: %s", logfile);

		tree_walk(print_file);

		fclose(logout);
	}

	tree_destroy();
	free(destdir);

	for (size_t i = 0; i < exclude_match_nr; i++)
		regfree(&exclude_match[i]);

	return EXIT_SUCCESS;
}
