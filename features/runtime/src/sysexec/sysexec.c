// SPDX-License-Identifier: GPL-3.0-or-later

#include "config.h"

#include <linux/magic.h>

#include <sys/syscall.h>
#include <sys/mount.h>
#include <sys/statfs.h>
#include <sys/stat.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <err.h>

/*
 * Because statfs.t_type can be int on some architectures, we have to cast the
 * const magic to the type, otherwise the compiler warns about signed/unsigned
 * comparison, because the magic can be 32 bit unsigned.
 */
#define F_TYPE_EQUAL(a, b) (a == (typeof(a)) b)

#define DIR_FLAGS	O_RDONLY | O_DIRECTORY | O_CLOEXEC

#ifndef pivot_root
static inline long pivot_root(const char *new_root, const char *put_old)
{
	return syscall(SYS_pivot_root, new_root, put_old);
}
#endif

static int is_dot_dir(struct dirent *ent)
{
	return ((ent->d_type == DT_DIR) &&
	        (ent->d_name[0] == '.' && ((ent->d_name[1] == '\0') ||
	                                   (ent->d_name[1] == '.' && ent->d_name[2] == '\0'))));
}

/* remove all files/directories below dirName -- don't cross mountpoints */
static int remove_recursive_at(int fd)
{
	struct stat root_sb;
	int rc = -1;
	DIR *dir = fdopendir(fd);

	if (!dir) {
		warn("failed to open directory");
		close(fd);
		return -1;
	}

	/* fdopendir() precludes us from continuing to use the input fd */
	int dir_fd = dirfd(dir);

	if (fstat(dir_fd, &root_sb)) {
		warn("stat failed");
		goto fail;
	}

	while(1) {
		struct dirent *d;
		int isdir = 0;

		errno = 0;
		if (!(d = readdir(dir))) {
			if (errno) {
				warn("failed to read directory");
				goto fail;
			}
			break;
		}

		if (is_dot_dir(d))
			continue;

		if (d->d_type == DT_DIR || d->d_type == DT_UNKNOWN) {
			struct stat dir_sb;

			if (fstatat(dir_fd, d->d_name, &dir_sb, AT_SYMLINK_NOFOLLOW)) {
				warn("stat of %s failed", d->d_name);
				continue;
			}

			/* skip if device is not the same */
			if (root_sb.st_dev != dir_sb.st_dev)
				continue;

			/* remove subdirectories */
			if (S_ISDIR(dir_sb.st_mode)) {
				int cfd;

				cfd = openat(dir_fd, d->d_name, DIR_FLAGS);

				if (cfd >= 0)
					remove_recursive_at(cfd);

				isdir = 1;
			}
		}

		if (unlinkat(dir_fd, d->d_name, isdir ? AT_REMOVEDIR : 0))
			warn("failed to unlink %s", d->d_name);
	}

	rc = 0;
fail:
	closedir(dir);
	return rc;
}

static bool is_temporary_fs(int fd)
{
	struct statfs stfs;

	if (fstatfs(fd, &stfs)) {
		warn("stat failed");
		return 0;
	}
	return (F_TYPE_EQUAL(stfs.f_type, RAMFS_MAGIC) ||
	        F_TYPE_EQUAL(stfs.f_type, TMPFS_MAGIC));
}

static void usage(int retcode)
{
	printf("Usage: %s <newrootdir> <init> [<args ...>]\n", program_invocation_short_name);
	exit(retcode);
}

int main(int argc, char *argv[])
{
	char *newroot, *initprog, **initargs;

	if (argc == 1) {
		usage(EXIT_SUCCESS);
	}

	if (argc < 3) {
		warnx("not enough arguments");
		usage(EXIT_FAILURE);
	}

	newroot  = argv[1];
	initprog = argv[2];
	initargs = &argv[2];

	if (!*newroot || !*initprog) {
		warnx("bad usage");
		usage(EXIT_FAILURE);
	}

	int old_root_fd = open("/", DIR_FLAGS);

	if (old_root_fd < 0)
		err(EXIT_FAILURE, "failed to open current root directory");

	int new_root_fd = open(newroot, DIR_FLAGS);

	if (new_root_fd < 0)
		err(EXIT_FAILURE, "failed to open new root directory: %s", newroot);

	/*
	 * Work-around for kernel design: the kernel refuses MS_MOVE if any file
	 * systems are mounted MS_SHARED. Hence remount them MS_PRIVATE here as
	 * a work-around.
	 *
	 * https://bugzilla.redhat.com/show_bug.cgi?id=847418
	 */
	if (mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL) < 0)
		err(EXIT_FAILURE, "failed to set / mount propagation to private");

	/*
	 * Note: implementation details of pivot_root may change with
	 * time. In order to ensure compatibility, the following points
	 * should be observed:
	 *
	 * - before calling pivot_root, the current directory of the
	 *   invoking process should point to the new root directory
	 * - use . as the first argument, and the _relative_ path of the
	 *   directory for the old root as the second argument
	 * - a chroot program must be available under the old and the
	 *   new root
	 * - chroot to the new root afterwards
	 * - use relative paths for dev/console in the exec command
	 *
	 * Documentation/admin-guide/initrd.rst#n251
	 */
	if (fchdir(new_root_fd))
		err(EXIT_FAILURE, "failed to change directory to %s", newroot);

	long ret = pivot_root(".", ".");
	if (ret >= 0) {
		/* unmount the upper of the two stacked file systems */
		if (umount2(".", MNT_DETACH))
			err(EXIT_FAILURE, "failed to unmount the old root");

	} else if (mount(".", "/", NULL, MS_MOVE, NULL)) {
		err(EXIT_FAILURE, "failed to move new root to /");
	}

	if (chroot(".") || chdir("/"))
		err(EXIT_FAILURE, "failed to chroot");

	if (is_temporary_fs(old_root_fd))
		remove_recursive_at(old_root_fd);

	execvp(initprog, initargs);
	err(EXIT_FAILURE, "failed to execute %s", initprog);
}
