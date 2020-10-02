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

struct file {
	struct file *prev;
	struct file *next;
	mode_t mode;
	size_t size;
	dev_t  dev;
	uid_t  uid;
	gid_t  gid;
	char   *src;
	char   *dst;
	char   *symlink;
	unsigned recursive:1;
	unsigned stat:1;
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
static int cwd;
static void *files = NULL;
static struct file *inqueue = NULL;

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
		err(EXIT_FAILURE, "calloc");

	new->src = (len < 0)
		? xstrdup(str)
		: xstrndup(str, (size_t) len);

	if (inqueue) {
		if (inqueue->prev)
			errx(EXIT_FAILURE, "bad queue head");
		inqueue->prev = new;
		new->next = inqueue;
	}

	inqueue = new;

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

static inline void fill_file(struct file *p, struct stat *sb)
{
	p->mode = sb->st_mode;
	p->size = (size_t) sb->st_size;
	p->dev  = sb->st_dev;
	p->uid  = sb->st_uid;
	p->gid  = sb->st_gid;
	p->stat = 1;
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
				return;
		}

		v.src = p->fts_path;

		if (tfind(&v, &files, compare))
			continue;

		struct file *f = add_list(p->fts_path, -1);
		fill_file(f, p->fts_statp);
	}

	fts_close(t);
}

static void mksock(const char *path, mode_t mode)
{
	struct sockaddr_un sun;

	if (strlen(path) >= sizeof(sun)) {
		errno = EINVAL;
		err(EXIT_FAILURE, "cannot bind socket: %s", path);
	}

	memset(&sun, 0, sizeof(sun));
	sun.sun_family = AF_UNIX;
	strcpy(sun.sun_path, path);

	int fd = socket(PF_UNIX, SOCK_STREAM, 0);
	if (fd < 0)
		err(EXIT_FAILURE, "cannot create socket: %s", path);

	if (fchmod(fd, mode))
		err(EXIT_FAILURE, "cannot set permissions of socket: %s", path);

	if (bind(fd, (struct sockaddr *) &sun, sizeof(sun)))
		err(EXIT_FAILURE, "cannot bind socket: %s", path);

	close (fd);
}

static const char *canonicalize_path(const char *path)
{
	static char pathbuf[PATH_MAX + 1];
	char *name;
	size_t len;
	int fd = -1;

	strcpy(pathbuf, path);

	name = strrchr(pathbuf, '/');
	if (!name)
		err(EXIT_FAILURE, "unable to get dirname: %s", path);
	*name++ = 0;
	name = xstrdup(name);

	if ((fd = open(pathbuf, O_PATH)) < 0)
		err(EXIT_FAILURE, "open: %s", pathbuf);

	if (fchdir(fd) < 0)
		err(EXIT_FAILURE, "chdir: %s", pathbuf);

	if (!getcwd(pathbuf, sizeof(pathbuf)))
		err(EXIT_FAILURE, "getcwd");

	if (fchdir(cwd) < 0)
		err(EXIT_FAILURE, "chdir");

	close(fd);

	len = strlen(pathbuf);
	strncat(pathbuf + len, "/",  sizeof(pathbuf) - 1 - len);
	len++;
	strncat(pathbuf + len, name, sizeof(pathbuf) - 1 - len);

	free(name);

	return pathbuf;
}

static const char *canonicalize_symlink(char *file, char *target)
{
	static char pathbuf[PATH_MAX + 1];
	char *name;
	size_t len;
	int fd;

	strcpy(pathbuf, file);

	name = strrchr(pathbuf, '/');
	if (name) {
		*name++ = 0;

		if ((fd = open(pathbuf, O_PATH)) < 0)
			err(EXIT_FAILURE, "open: %s", pathbuf);
		if (fchdir(fd) < 0)
			err(EXIT_FAILURE, "chdir: %s", pathbuf);
		close(fd);
	}

	strcpy(pathbuf, target);

	name = strrchr(pathbuf, '/');
	if (name) {
		*name++ = 0;

		if ((fd = open(pathbuf, O_PATH)) < 0)
			err(EXIT_FAILURE, "open: %s", pathbuf);
		if (fchdir(fd) < 0)
			err(EXIT_FAILURE, "chdir: %s", pathbuf);
		close(fd);
	} else {
		name = target;
	}
	name = xstrdup(name);

	if (!getcwd(pathbuf, sizeof(pathbuf)))
		err(EXIT_FAILURE, "getcwd");

	len = strlen(pathbuf);

	strncat(pathbuf + len, "/",  sizeof(pathbuf) - 1 - len);
	len++;
	strncat(pathbuf + len, name, sizeof(pathbuf) - 1 - len);
	free(name);

	if (fchdir(cwd) < 0)
		err(EXIT_FAILURE, "chdir");

	return pathbuf;
}

enum ftype {
	FTYPE_ERROR = 0,
	FTYPE_IGNORE,
	FTYPE_DATA,
	FTYPE_TEXT_SCRIPT,
	FTYPE_ELF_STATIC,
	FTYPE_ELF_DYNAMIC,
};

static enum ftype elf_file(int fd)
{
	int is_dynamic;
	Elf *e;
	Elf_Scn *scn;
	size_t shstrndx;
	int rc = FTYPE_ERROR;

	if ((e = elf_begin(fd, ELF_C_READ, NULL)) == NULL) {
		warnx("elf_begin: %s", elf_errmsg(-1));
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
		warnx("elf_getshdrstrndx: %s", elf_errmsg(-1));
		goto end;
	}

	is_dynamic = 0;

	for (scn = NULL; (scn = elf_nextscn(e, scn)) != NULL;) {
		GElf_Shdr  shdr;

		if (gelf_getshdr(scn , &shdr) != &shdr) {
			warnx("gelf_getshdr: %s", elf_errmsg(-1));
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
		if (errno == EACCES || errno == EPERM) {
			ret = 0;
			goto end;
		}
		warn("open: %s", filename);
		return -1;
	}

	if (pread(fd, buf, sizeof(buf), 0) < 0) {
		warn("read: %s", filename);
		goto end;
	}

	buf[LINE_MAX - 1] = 0;

	if (buf[0] == '#' &&
	    buf[1] == '!') {
		char *p, *q;

		for (p = &buf[2]; *p && isspace(*p); p++);
		for (q = p; *q && (!isspace(*q)); q++);
		*q = '\0';

		add_list(p, -1);

		ret = 0;
		goto end;
	}

	if (buf[0] == ELFMAG[0] &&
	    buf[1] == ELFMAG[1] &&
	    buf[2] == ELFMAG[2] &&
	    buf[3] == ELFMAG[3] &&
	    elf_file(fd) == FTYPE_ELF_DYNAMIC) {
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
	if (!p->stat) {
		struct stat sb;

		if (lstat(p->src, &sb) < 0)
			err(EXIT_FAILURE, "lstat: %s", p->src);

		fill_file(p, &sb);
	}

	p->dst = p->src;

	if (prefix) {
		size_t dst_len = strlen(p->dst);
		if (dst_len >= prefix_len && p->dst[prefix_len] == '/' && !strncmp(p->dst, prefix, prefix_len - 1))
			p->dst += prefix_len;
		else if (!strcmp(p->dst, prefix))
			p->dst = (char *) "";
	}

	add_parent_directory(p->src);

	if (S_IFDIR == (p->mode & S_IFMT)) {
		if (p->recursive)
			process_directory(p->src);
		return;
	}

	if (S_IFLNK == (p->mode & S_IFMT)) {
		static char symlink[PATH_MAX + 1];

		ssize_t linklen = readlink(p->src, symlink, sizeof(symlink));
		if (linklen >= 0) {
			symlink[linklen] = 0;

			p->symlink = xstrdup(symlink);

			const char *s = canonicalize_symlink(p->src, p->symlink);

			struct file *f = add_list(s, -1);
			f->recursive = 1;

			return;
		}

		warn("readlink: %s", p->src);
		return;
	}

	if (S_IFREG == (p->mode & S_IFMT)) {
		if (process_regular_file(p->src) < 0)
			warnx("failed to read regular file: %s", p->src);
		return;
	}
}

static FILE *logout;

static void print_file(struct file *p)
{
	char type;

	switch (p->mode & S_IFMT) {
		case S_IFBLK:  type = 'b'; break;
		case S_IFCHR:  type = 'c'; break;
		case S_IFDIR:  type = 'd'; break;
		case S_IFIFO:  type = 'p'; break;
		case S_IFLNK:  type = 'l'; break;
		case S_IFREG:  type = 'f'; break;
		case S_IFSOCK: type = 's'; break;
		default:       type = '?'; break;
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
	int sfd, dfd;

	strncpy(install_path + destdir_len, p->dst, sizeof(install_path) - destdir_len - 1);

	if (S_IFDIR == (p->mode & S_IFMT)) {
		errno = 0;
		if (mkdir(install_path, p->mode) < 0) {
			if (errno != EEXIST) {
				err(EXIT_FAILURE, "mkdir: %s", install_path);
			} else if (verbose) {
				warnx("skip (directory): %s", p->dst);
			}
		} else if (verbose) {
			warnx("install (directory): %s", p->dst);
		}
		goto chown;
	}

	errno = 0;
	if (force && remove(install_path) < 0 && errno != ENOENT)
		err(EXIT_FAILURE, "remove: %s", install_path);

	if (S_IFBLK == (p->mode & S_IFMT) ||
			S_IFCHR == (p->mode & S_IFMT)) {
		errno = 0;
		if (mknod(install_path, p->mode, p->dev)) {
			if (errno != EEXIST) {
				err(EXIT_FAILURE, "mknod: %s", install_path);
			} else if (verbose) {
				warnx("skip (divice file): %s", p->dst);
			}
		} else if (verbose) {
			warnx("install (divice file): %s", p->dst);
		}
		goto chown;
	}

	if (S_IFLNK == (p->mode & S_IFMT)) {
		errno = 0;
		if (symlink(p->symlink, install_path) < 0) {
			if (errno != EEXIST) {
				err(EXIT_FAILURE, "symlink: %s", install_path);
			} else if (verbose) {
				warnx("skip (symlink): %s", p->dst);
			}
		} else if (verbose) {
			warnx("install (symlink): %s", p->dst);
		}
		goto chown;
	}

	if (S_IFIFO == (p->mode & S_IFMT)) {
		errno = 0;
		if (mkfifo(install_path, p->mode) < 0) {
			if (errno != EEXIST) {
				err(EXIT_FAILURE, "mkfifo: %s", install_path);
			} else if (verbose) {
				warnx("skip (fifo): %s", p->dst);
			}
		} else if (verbose) {
			warnx("install (fifo): %s", p->dst);
		}
		goto chown;
	}

	if (S_IFSOCK == (p->mode & S_IFMT)) {
		errno = 0;
		mksock(install_path, p->mode);
		if (verbose)
			warnx("install (socket): %s", install_path);
		goto chown;
	}

	if (S_IFREG != (p->mode & S_IFMT))
		errx(EXIT_FAILURE, "not implemented (mode=%o): %s", p->mode, p->dst);

	if (!access(install_path, X_OK)) {
		if (verbose)
			warnx("skip (file): %s", p->dst);
		goto chown;
	}

	errno = 0;
	if ((dfd = creat(install_path, p->mode)) < 0) {
		err(EXIT_FAILURE, "creat: %s", install_path);
	} else if (verbose) {
		warnx("install (file): %s", p->dst);
	}

	errno = 0;
	if ((sfd = open(p->src, O_RDONLY)) < 0)
		err(EXIT_FAILURE, "open: %s", p->src);

	posix_fadvise(sfd, 0, (off_t) p->size, POSIX_FADV_SEQUENTIAL);
	posix_fallocate(dfd, 0, (off_t) p->size);

	ssize_t ret;
	size_t len = p->size;

	if (!use_copy_file_range)
		goto fallback_sendfile;
	do {
		errno = 0;
		ret = copy_file_range(sfd, NULL, dfd, NULL, len, 0);
		if (ret < 0) {
			if (errno == EXDEV) {
				use_copy_file_range = 0;
				goto fallback_sendfile;
			}
			err(EXIT_FAILURE, "copy_file_range: %s -> %s", p->src, p->dst);
		}
		len -= (size_t) ret;
	} while (len > 0 && ret > 0);
finish:
	close(sfd);
	close(dfd);
chown:
	errno = 0;
	if (lchown(install_path, p->uid, p->gid) < 0 && errno != EPERM)
		err(EXIT_FAILURE, "chown: %s", install_path);
	return;

fallback_sendfile:
	if (!use_sendfile)
		goto fallback_readwrite;

	lseek(sfd, 0, SEEK_SET);
	lseek(dfd, 0, SEEK_SET);

	len = p->size;
	do {
		errno = 0;
		ret = sendfile(dfd, sfd, NULL, len);
		if (ret < 0) {
			if (errno == EINVAL || errno == ENOSYS) {
				use_sendfile = 0;
				goto fallback_readwrite;
			}
			err(EXIT_FAILURE, "sendfile: %s -> %s", p->src, p->dst);
		}
		len -= (size_t) ret;
	} while (len > 0 && ret > 0);

	goto finish;

fallback_readwrite:
	lseek(sfd, 0, SEEK_SET);
	lseek(dfd, 0, SEEK_SET);

	char buf[BUFSIZ];
	len = p->size;
	do {
		ret = TEMP_FAILURE_RETRY(read(sfd, buf, sizeof(buf)));
		if (ret < 0)
			err(EXIT_FAILURE, "read: %s", p->src);

		ret = TEMP_FAILURE_RETRY(write(dfd, buf, (size_t) ret));
		if (ret < 0)
			err(EXIT_FAILURE, "write: %s", p->dst);

		len -= (size_t) ret;
	} while (len > 0 && ret > 0);

	goto finish;
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

	cwd = open(".", O_PATH);
	if (!cwd)
		err(EXIT_FAILURE, "unable to open current directory");

	for (int i = optind; i < argc; i++) {
		const char *s = canonicalize_path(argv[i]);
		struct file *f = add_list(s, -1);
		f->recursive = 1;
	}

	while (inqueue != NULL) {
		struct file *queue = inqueue;

		while (queue != NULL) {
			void *ptr;
			struct file *next = queue->next;

			ptr = tsearch(queue, &files, compare);
			if (!ptr)
				err(EXIT_FAILURE, "tsearch");

			if ((*(struct file **)ptr) == queue) {
				process(*(struct file **) ptr);
				del_list(queue);
			} else {
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
		strncpy(install_path, destdir, sizeof(install_path) - 1);
		install_path[destdir_len] = 0;

		umask(0);

		twalk_r(files, walk_action, install_file);
	}

	if (logfile) {
		if ((logout = fopen(logfile, "a")) == NULL)
			err(EXIT_FAILURE, "open: %s", logfile);

		twalk_r(files, walk_action, print_file);

		fclose(logout);
	}

	tdestroy(files, free_file);
	return EXIT_SUCCESS;
}
