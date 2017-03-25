#include <stdio.h>
#include <unistd.h>

#include "uevent-logging.h"
#include "uevent-worker.h"

pid_t
spawn_worker(char *path, char **argv)
{
	pid_t pid;

	if ((pid = fork()) == -1)
		fatal("fork: %m");

	if (pid > 0)
		return pid;

	if (execv(path, argv) < 0)
		fatal("execv: %m");

	return -1;
}
