#include <linux/limits.h>

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/param.h>

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <error.h>

#include "container.h"

static int
get_open_max(void)
{
	long int i = sysconf(_SC_OPEN_MAX);

	if (i < NR_OPEN)
		i = NR_OPEN;
	if (i > INT_MAX)
		i = INT_MAX;

	return (int) i;
}

void
sanitize_fds(void)
{
	struct stat st;
	int fd, max_fd;

	umask(0);

	for (fd = STDIN_FILENO; fd <= STDERR_FILENO; ++fd) {
		if (fstat(fd, &st) < 0)
			error(EXIT_FAILURE, errno, "fstat");
	}

	max_fd = get_open_max();

	for (; fd < max_fd; ++fd)
		(void) close(fd);

	errno = 0;
}

void
cloexec_fds(void)
{
	int fd, max_fd = get_open_max();

	/* Set close-on-exec flag on all non-standard descriptors. */
	for (fd = STDERR_FILENO + 1; fd < max_fd; ++fd) {
		int flags = fcntl(fd, F_GETFD, 0);

		if (flags < 0)
			continue;

		int newflags = flags | FD_CLOEXEC;

		if (flags != newflags && fcntl(fd, F_SETFD, newflags))
			error(EXIT_FAILURE, errno, "fcntl F_SETFD");
	}

	errno = 0;
}

void
open_map(char *filename, struct mapfile *f)
{
	struct stat sb;

	if ((f->fd = open(filename, O_RDONLY, O_CLOEXEC)) < 0)
		error(EXIT_FAILURE, errno, "open: %s", filename);

	if (fstat(f->fd, &sb) < 0)
		error(EXIT_FAILURE, errno, "fstat: %s", filename);

	if (!sb.st_size) {
		close(f->fd);
		error(EXIT_SUCCESS, 0, "file %s is empty", filename);
		return;
	}

	f->size = (size_t) sb.st_size;

	if ((f->map = mmap(NULL, f->size, PROT_READ, MAP_PRIVATE, f->fd, 0)) == MAP_FAILED)
		error(EXIT_FAILURE, errno, "mmap: %s", filename);

	f->filename = filename;
}

void
close_map(struct mapfile *f)
{
	if (!f->filename)
		return;

	if (munmap(f->map, f->size) < 0)
		error(EXIT_FAILURE, errno, "munmap: %s", f->filename);

	close(f->fd);
	f->filename = NULL;
}
