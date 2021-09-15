#define _GNU_SOURCE

#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/sendfile.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <err.h>
#include <sysexits.h>
#include <fts.h>
#include <fcntl.h>
#include <ctype.h>
#include <search.h>

#include <gelf.h>

#include "config.h"

struct file {
	struct file *prev;
	struct file *next;
	struct stat stat;
	char *src;
	char *dst;
	char *symlink;
	unsigned recursive:1;
	unsigned installed:1;
};

static const char *progname = NULL;

static char *destdir = NULL;
static size_t destdir_len = 0;
static char *prefix = NULL;
static size_t prefix_len = 0;
static char *logfile = NULL;
static int verbose = 0;
static int dry_run = 0;
static int force = 0;
static void *files = NULL;
static struct file *inqueue = NULL;
static size_t installed = 0;
static size_t queue_nr = 0;

static char *xstrdup(const char *s)
{
	char *x = strdup(s);
	if (!x)
		err(EX_OSERR, "strdup");
	return x;
}

static char *xstrndup(const char *s, size_t n)
{
	char *x = strndup(s, n);
	if (!x)
		err(EX_OSERR, "strndup");
	return x;
}

static struct file *add_list(const char *str, ssize_t len)
{
	struct file *new;

	new = calloc(1, sizeof(*new));
	if (!new)
		err(EX_OSERR, "calloc");

	new->src = (len < 0)
	           ? xstrdup(str)
	           : xstrndup(str, (size_t) len);

	if (verbose > 1)
		warnx("add to list: %s", new->src);

	if (inqueue) {
		if (inqueue->prev)
			errx(EX_SOFTWARE, "bad queue head");
		inqueue->prev = new;
		new->next = inqueue;
	}

	inqueue = new;
	queue_nr++;

	return new;
}

static void del_list(struct file *ptr)
{
	if (!ptr)
		return;
	if (ptr->prev)
		ptr->prev->next = ptr->next;
	if (ptr->next)
		ptr->next->prev = ptr->prev;
	if (inqueue == ptr)
		inqueue = NULL;
	queue_nr--;
}

static void add_parent_directory(char *path)
{
	if (!path || (prefix && !strcmp(path, prefix)))
		return;

	char *p = strrchr(path, '/');
	if (!p || p == path)
		return;

	add_list(path, p - path);
}

static int compare(const void *a, const void *b)
{
	return strcmp(((struct file *)a)->src, ((struct file *)b)->src);
}

static inline void fill_stat(struct file *p, struct stat *sb)
{
	memcpy(&p->stat, sb, sizeof(p->stat));
}

static void process_directory(char *path)
{
	FTS *t = NULL;
	struct file v = { 0 };
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

		v.src = p->fts_path;

		if (tfind(&v, &files, compare))
			continue;

		struct file *f = add_list(p->fts_path, -1);
		fill_stat(f, p->fts_statp);
	}

	fts_close(t);
}

static int mksock(const char *path)
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

#define IS_DIR_SEPARATOR(c) ((c) == '/')
#define IS_NUL_OR_DIR_SEPARATOR(c) ((c) == '\0' || (c) == '/')
#define SKIP_DIR_SEPARATOR(s_) while (IS_DIR_SEPARATOR(*(s_))) (s_)++

static void canonicalize_path(const char *s, char *d)
{
	if (*s == '\0') {
		*d = '\0';
		return;
	}

	const char *const orig_d = d;

	if (IS_DIR_SEPARATOR (*s)) {
		*d++ = *s++;
		SKIP_DIR_SEPARATOR(s);
	}

	const char *const droot = d;

	while (*s) {
		/* At this point, we're always at the beginning of a path segment.  */
		if (s[0] == '.' && IS_NUL_OR_DIR_SEPARATOR(s[1])) {
			s++;
			SKIP_DIR_SEPARATOR(s);
		}

		else if (s[0] == '.' && s[1] == '.' && IS_NUL_OR_DIR_SEPARATOR(s[2])) {
			char *pre = d - 1; /* includes slash */
			while (droot < pre && IS_DIR_SEPARATOR(*pre))
				pre--;
			if (droot <= pre && ! IS_DIR_SEPARATOR(*pre)) {
				while (droot < pre && ! IS_DIR_SEPARATOR(*pre))
					pre--;
				/* If droot < pre, then pre points to the slash.  */
				if (droot < pre)
					pre++;
				if (pre < droot || (pre + 3 == d && pre[0] == '.' && pre[1] == '.')) {
					/* Append ".." segment.  */
					*d++ = *s++;
					*d++ = *s++;
				} else {
					d = pre;
					s += 2;
					SKIP_DIR_SEPARATOR(s);
				}
			} else {
				/* Append ".." segment.  */
				*d++ = *s++;
				*d++ = *s++;
			}
		} else {
			while (*s && ! IS_DIR_SEPARATOR(*s))
				*d++ = *s++;
		}

		if (IS_DIR_SEPARATOR(*s)) {
			*d++ = *s++;
			SKIP_DIR_SEPARATOR(s);
		}
	}

	while (droot < d && IS_DIR_SEPARATOR(d[-1]))
		--d;

	if (d == orig_d)
		*d++ = '.';

	*d = '\0';
}

static void canonicalize_symlink(char *file, char *target, char *d)
{
	char pathbuf[PATH_MAX + 1];
	char *p;

	if (verbose > 2)
		warnx("canonicalize_symlink: %s", file);

	if (target && target[0] == '/') {
		canonicalize_path(target, d);
		return;
	}

	strncpy(pathbuf, file, sizeof(pathbuf) - 1);
	if ((p = strrchr(pathbuf, '/'))) {
		p++;
		*p = 0;
	}
	strncat(pathbuf, target, sizeof(pathbuf) - 1 - strlen(pathbuf));

	canonicalize_path(pathbuf, d);
}

enum ftype {
	FTYPE_ERROR = 0,
	FTYPE_IGNORE,
	FTYPE_DATA,
	FTYPE_TEXT_SCRIPT,
	FTYPE_ELF_STATIC,
	FTYPE_ELF_DYNAMIC,
};

static enum ftype elf_file(const char *filename, int fd)
{
	int is_dynamic;
	Elf *e;
	Elf_Scn *scn;
	size_t shstrndx;
	int rc = FTYPE_ERROR;

	if ((e = elf_begin(fd, ELF_C_READ, NULL)) == NULL) {
		warnx("%s: elf_begin: %s", filename, elf_errmsg(-1));
		goto err;
	}

	switch (elf_kind(e)) {
		case  ELF_K_NONE:
		case  ELF_K_AR:
		case ELF_K_COFF:
		case  ELF_K_ELF:
			break;
		default:
			goto end;
	}

	if (elf_getshdrstrndx(e, &shstrndx) != 0) {
		warnx("%s: elf_getshdrstrndx: %s", filename, elf_errmsg(-1));
		goto end;
	}

	is_dynamic = 0;

	for (scn = NULL; (scn = elf_nextscn(e, scn)) != NULL;) {
		GElf_Shdr  shdr;

		if (gelf_getshdr(scn, &shdr) != &shdr) {
			warnx("%s: gelf_getshdr: %s", filename, elf_errmsg(-1));
			goto end;
		}

		if (shdr.sh_type == SHT_DYNAMIC) {
			is_dynamic = 1;
			break;
		}
	}

	rc = (is_dynamic ? FTYPE_ELF_DYNAMIC : FTYPE_ELF_STATIC);
end:
	elf_end(e);
err:
	return rc;
}

static int shared_object_dependencies(const char *filename)
{
	FILE *pfd;
	char *line = NULL;
	size_t len = 0;
	ssize_t n;
	char command[PATH_MAX + 10];

	snprintf(command, sizeof(command), "ldd %s 2>&1", filename);

	pfd = popen(command, "r");
	if (!pfd) {
		warn("popen(ldd): %s", filename);
		return -1;
	}

	while ((n = getline(&line, &len, pfd)) != -1) {
		char *p;

		if (line[n - 1] == '\n')
			line[n - 1] = '\0';

		p = strstr(line, "(0x");
		if (!p)
			continue;
		*p-- = '\0';

		while (line != p && isspace(*p))
			*p-- = '\0';

		p = strstr(line, " => ");
		if (p)
			p += 4;
		else
			p = line;

		while (*p != '\0' && isspace(*p))
			*p++ = '\0';

		if (*p != '/')
			continue;

		if (verbose > 1)
			warnx("shared object '%s' depends on '%s'", filename, p);

		add_list(p, -1);
	}

	free(line);
	pclose(pfd);

	return 0;
}

static int process_regular_file(const char *filename)
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

	if (!access(filename, X_OK) &&
	    buf[0] == '#' &&
	    buf[1] == '!') {
		char *p, *q;

		for (p = &buf[2]; *p && isspace(*p); p++);
		for (q = p; *q && (!isspace(*q)); q++);
		*q = '\0';

		if (verbose > 1)
			warnx("shell script '%s' uses the '%s' interpreter", filename, p);

		add_list(p, -1);

		ret = 0;
		goto end;
	}

	if (buf[0] == ELFMAG[0] &&
	    buf[1] == ELFMAG[1] &&
	    buf[2] == ELFMAG[2] &&
	    buf[3] == ELFMAG[3] &&
	    elf_file(filename, fd) == FTYPE_ELF_DYNAMIC) {
		ret = shared_object_dependencies(filename);
		goto end;
	}

	ret = 0;
end:
	close(fd);
	return ret;
}

static void process(struct file *p)
{
	if (!p->stat.st_ino) {
		struct stat sb;

		if (lstat(p->src, &sb) < 0)
			err(EXIT_FAILURE, "lstat: %s", p->src);

		fill_stat(p, &sb);
	}

	add_parent_directory(p->src);

	if (S_IFDIR == (p->stat.st_mode & S_IFMT)) {
		if (p->recursive)
			process_directory(p->src);
		return;
	}

	if (S_IFLNK == (p->stat.st_mode & S_IFMT)) {
		static char symlink[PATH_MAX + 1];

		ssize_t linklen = readlink(p->src, symlink, sizeof(symlink));
		if (linklen >= 0) {
			symlink[linklen] = 0;
			p->symlink = xstrdup(symlink);
			symlink[0] = 0;

			canonicalize_symlink(p->src, p->symlink, symlink);
			if (*symlink) {
				if (verbose > 1)
					warnx("symlink '%s' points to '%s'", p->src, symlink);

				add_list(symlink, -1);
				return;
			}

			warnx("unable to resolve symlink '%s' -> '%s'", p->src, p->symlink);
			return;
		}

		warn("readlink: %s", p->src);
		return;
	}

	if (S_IFREG == (p->stat.st_mode & S_IFMT)) {
		if (process_regular_file(p->src) < 0)
			warnx("failed to read regular file: %s", p->src);
		return;
	}
}

static FILE *logout;

static void print_file(struct file *p)
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

	fprintf(logout, "%c\t%s\t%s%s%s\t%s\n",
	        type,
	        p->src,
	        destdir, (p->dst[0] == '/' ? "" : "/"), p->dst,
	        (p->symlink ? p->symlink : ""));
}

static void free_file(void *ptr)
{
	struct file *p = ptr;
	free(p->src);
	free(p->symlink);
	free(p);
}

static char install_path[PATH_MAX + 1];
static int use_copy_file_range = 1;
static int use_sendfile = 1;

static void install_file(struct file *p)
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
	if ((dfd = creat(install_path, p->stat.st_mode)) < 0) {
		if (errno == ENOENT)
			return;
		err(EX_CANTCREAT, "creat: %s", install_path);
	}

	if ((sfd = open(p->src, O_RDONLY)) < 0)
		err(EX_NOINPUT, "open: %s", p->src);

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
	p->installed = 1;
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

static void apply_permissions(struct file *p)
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

static void walk_action(const void *nodep, VISIT which, void *closure)
{
	struct file *p;
	void (*file_handler)(struct file *p) = closure;

	switch (which) {
		case preorder:
			break;
		case postorder:
			p = *(struct file **) nodep;
			file_handler(p);
			break;
		case endorder:
			break;
		case leaf:
			p = *(struct file **) nodep;
			file_handler(p);
			break;
	}
}

#ifndef HAVE_TWALK_R
static void *twalk_r_closure;
static void (*twalk_r_action)(const void *nodep, VISIT which, void *closure);

static void twalk_handler(const void *nodep, VISIT which,
                          int depth __attribute__((unused)))
{
	twalk_r_action(nodep, which, twalk_r_closure);
}

static void twalk_r(const void *root,
                    void (*action)(const void *nodep, VISIT which, void *closure),
                    void *closure)
{
	twalk_r_action  = action;
	twalk_r_closure = closure;
	twalk(root, twalk_handler);
}
#endif

#ifndef HAVE_TDESTROY
static inline void tdestroy(
        void *root __attribute__((unused)),
        void (*free_node)(void *nodep) __attribute__((unused)))
{}
#endif

static void show_help(int rc)
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

static void show_version(void)
{
	fprintf(stdout,
	        "%1$s version " PACKAGE_VERSION "\n"
	        "Written by Alexey Gladkov.\n"
	        "\n"
	        "Copyright (C) 2012-2020  Alexey Gladkov <gladkov.alexey@gmail.com>\n"
	        "This is free software; see the source for copying conditions.  There is NO\n"
	        "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n",
	        progname
	       );
	exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
	const char *optstring = "fnl:r:vVh";
	const struct option longopts[] = {
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

	destdir = argv[optind++];
	destdir_len = strlen(destdir);

	if (optind == argc)
		errx(EX_USAGE, "more arguments required");

	if (elf_version(EV_CURRENT) == EV_NONE)
		errx(EXIT_FAILURE, "ELF library initialization failed: %s", elf_errmsg(-1));

	for (int i = optind; i < argc; i++) {
		char pathbuf[PATH_MAX + 1];
		struct file *f;

		canonicalize_path(argv[i], pathbuf);

		f = add_list(pathbuf, -1);
		f->recursive = 1;
	}

	strncpy(install_path, destdir, sizeof(install_path) - 1);
	install_path[destdir_len] = 0;

	while (inqueue != NULL) {
		struct file *queue = inqueue;

		while (queue != NULL) {
			void *ptr;
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
					del_list(queue);
					free_file(queue);
					queue = next;
					continue;
				}
			}

			ptr = tsearch(queue, &files, compare);
			if (!ptr)
				err(EX_OSERR, "tsearch");

			if ((*(struct file **)ptr) == queue) {
				process(*(struct file **) ptr);
				del_list(queue);
			} else {
				if (verbose > 1)
					warnx("'%s' has already been processed so skip it", queue->src);
				del_list(queue);
				free_file(queue);
			}

			queue = next;
		}
	}

	if (dry_run) {
		if (verbose > 1)
			warnx("dry run only ...");
		logout = stdout;
		twalk_r(files, walk_action, print_file);
	} else {
		if (verbose > 1)
			warnx("copying files ...");

		umask(0);

		while (1) {
			size_t prev_installed = installed;

			twalk_r(files, walk_action, install_file);

			if (prev_installed == installed)
				break;
		}

		if (queue_nr > installed) {
			logout = stdout;
			twalk_r(files, walk_action, print_file);
			errx(EXIT_FAILURE, "unable to create the files listed above");
		}

		twalk_r(files, walk_action, apply_permissions);
	}

	if (logfile) {
		if ((logout = fopen(logfile, "a")) == NULL)
			err(EX_CANTCREAT, "open: %s", logfile);

		twalk_r(files, walk_action, print_file);

		fclose(logout);
	}

	tdestroy(files, free_file);
	return EXIT_SUCCESS;
}
