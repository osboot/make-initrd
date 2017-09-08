#include <sys/types.h>
#include <sys/wait.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <error.h>

#include "container.h"

extern int verbose;

int
run_hook(const char *type)
{
	pid_t pid;
	char name[256];
	char *value;

	snprintf(name, 255, "CONTAINER_HOOK_%s", type);

	if (!(value = getenv(name)))
		return -2;

	if (access(value, R_OK | X_OK) < 0) {
		error(EXIT_SUCCESS, errno, "%s: access", name);
		return -2;
	}

	if (verbose)
		dprintf(STDERR_FILENO, "HOOK(%s): %s\n", type, value);

	errno = 0;
	if ((pid = fork()) < 0) {
		error(EXIT_SUCCESS, errno, "hook %s failed: fork", name);
		return -1;
	} else if (pid > 0)
		return pid;

	sanitize_fds();

	execlp(value, value, NULL);
	error(EXIT_FAILURE, errno, "execvp");

	return -1;
}
