/* SPDX-License-Identifier: GPL-2.0-or-later */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>

#include "rd/memory.h"
#include "rd/logging.h"
#include "ueventd.h"

static void event_handler(struct watch *queue, char *path) __attribute__((nonnull(1, 2)));
static void move_files(char *srcdir, char *dstdir)         __attribute__((nonnull(1, 2)));
static void signal_unhandled_events(struct watch *queue)   __attribute__((nonnull(1)));

void event_handler(struct watch *queue, char *path)
{
	char *argv[] = { handler_file, path, NULL };
	pid_t pid = vfork();

	if (pid < 0) {
		rd_err("fork: %m");

	} else if (!pid) {
		execve(argv[0], argv, environ);
		rd_fatal("exec: %s: %m", argv[0]);
	} else {
		int status = 0;

		if (waitpid_retry(pid, &status, 0) != pid || !WIFEXITED(status))
			rd_info("%s: session=%lu: %s failed",
			        queue->q_name, session, handler_file);
		else
			rd_info("%s: session=%lu: %s finished with return code %d",
			        queue->q_name, session, handler_file, WEXITSTATUS(status));
	}
}

void move_files(char *srcdir, char *dstdir)
{
	struct dirent *ent;
	int srcfd, dstfd;

	if ((srcfd = open(srcdir, O_RDONLY | O_CLOEXEC)) < 0)
		rd_fatal("open: %s: %m", srcdir);

	errno = 0;
	if ((dstfd = open(dstdir, O_RDONLY | O_CLOEXEC)) < 0) {
		if (errno == ENOENT) {
			if (mkdir(dstdir, 0755) < 0)
				rd_fatal("mkdir: %s: %m", dstdir);
			dstfd = open(dstdir, O_RDONLY | O_CLOEXEC);
		}
		if (dstfd < 0)
			rd_fatal("open: %s: %m", dstdir);
	}

	DIR *d = fdopendir(srcfd);
	if (!d)
		rd_fatal("fdopendir: %m");

	while ((ent = xreaddir(d, srcdir)) != NULL) {
		if (ent->d_name[0] != '.' && ent->d_type == DT_REG &&
		    renameat(srcfd, ent->d_name, dstfd, ent->d_name) < 0)
			rd_fatal("rename `%s/%s' -> `%s/%s': %m", srcdir, ent->d_name, dstdir, ent->d_name);
	}

	closedir(d);
	close(dstfd);
}

void signal_unhandled_events(struct watch *queue)
{
	ssize_t len;

	len = write_loop(queue->q_parentfd,
	                 (char *) &queue->q_watchfd, sizeof(queue->q_watchfd));
	if (len < 0)
		rd_err("write(pipe): %m");

	rd_info("%s: session=%lu: retry with the events remaining in the queue", queue->q_name, session);
}

void process_events(struct watch *queue)
{
	rd_info("%s: session=%lu: processing events", queue->q_name, session);

	char *numenv;

	rd_asprintf_or_die(&numenv, "SESSION=%lu", session);
	putenv(numenv);

	char *srcdir, *dstdir;

	rd_asprintf_or_die(&srcdir, "%s/%s", filter_dir, queue->q_name);
	rd_asprintf_or_die(&dstdir, "%s/%s", uevent_dir, queue->q_name);

	move_files(srcdir, dstdir);

	if (!empty_directory(dstdir)) {
		event_handler(queue, dstdir);

		if (!empty_directory(dstdir))
			signal_unhandled_events(queue);
	}

	free(srcdir);
	free(dstdir);

	exit(0);
}
