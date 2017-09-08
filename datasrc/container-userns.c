#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <error.h>

#include "container.h"

#define PROC_ROOT "/proc"

void
map_id(const char *filename, const pid_t pid, const uint64_t from, const uint64_t to)
{
	char *file = NULL;
	int fd;

	xasprintf(&file, PROC_ROOT "/%d/%s", pid, filename);

	if ((fd = open(file, O_WRONLY)) < 0)
		error(EXIT_FAILURE, errno, "open: %s", file);

	if (dprintf(fd, "%lu %lu 1", from, to) < 0)
		dprintf(STDERR_FILENO, "unable to write to %s\n", file);

	close(fd);
	xfree(file);
}

void
setgroups_control(const pid_t pid, const char *value)
{
	char *file;
	int fd;

	xasprintf(&file, PROC_ROOT "/%d/setgroups", pid);

	if ((fd = open(file, O_WRONLY)) < 0)
		error(EXIT_FAILURE, errno, "open: %s", file);

	if (dprintf(fd, "%s", value) < 0)
		dprintf(STDERR_FILENO, "unable to write to %s\n", file);

	close(fd);
	xfree(file);
}
