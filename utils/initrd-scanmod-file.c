#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>

#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <error.h>
#include <errno.h>

#include "initrd-scanmod.h"

int
open_map(const char *filename, struct mapfile *f, int quiet)
{
	struct stat sb;

	if ((f->fd = open(filename, O_RDONLY | O_CLOEXEC)) < 0) {
		error(EXIT_SUCCESS, errno, "open: %s", filename);
		return -1;
	}

	if (fstat(f->fd, &sb) < 0) {
		error(EXIT_SUCCESS, errno, "fstat: %s", filename);
		return -1;
	}

	f->size = (size_t) sb.st_size;

	if (!sb.st_size) {
		close(f->fd);
		f->fd = -1;
		if (!quiet)
			error(EXIT_SUCCESS, 0, "file %s is empty", filename);
		return 0;
	}

	if ((f->map = mmap(NULL, f->size, PROT_READ, MAP_PRIVATE, f->fd, 0)) == MAP_FAILED) {
		error(EXIT_SUCCESS, errno, "mmap: %s", filename);
		return -1;
	}

	f->filename = filename;
	return 0;
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
