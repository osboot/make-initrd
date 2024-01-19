/* SPDX-License-Identifier: GPL-2.0-or-later */

#include <linux/kd.h>
#include <sys/ioctl.h>

#include <unistd.h>
#include <fcntl.h>

#include "rd/logging.h"
#include "rd/console.h"

static const char *conspath[] = {
	"/dev/console",
	"/dev/systty",
	"/dev/tty0",
	"/dev/tty",
	"/dev/vc/0",
	"/proc/self/fd/0",
	NULL
};

static inline int is_a_console(int fd)
{
	char arg = 0;
	return (isatty(fd) &&
	        !ioctl(fd, KDGKBTYPE, &arg) &&
	        ((arg == KB_101) || (arg == KB_84)));
}

static int open_a_console(const char *fnam)
{
	int fd;

	/*
	 * For ioctl purposes we only need some fd and permissions
	 * do not matter.
	 */
	fd = open(fnam, O_RDWR);
	if (fd < 0)
		fd = open(fnam, O_WRONLY);
	if (fd < 0)
		fd = open(fnam, O_RDONLY);
	if (fd < 0)
		return -1;
	return fd;
}

int rd_open_console(void)
{
	int fd;

	for (int i = 0; conspath[i]; i++) {
		if ((fd = open_a_console(conspath[i])) >= 0) {
			if (is_a_console(fd)) {
				rd_dbg("using %s", conspath[i]);
				return fd;
			}
			close(fd);
		}
	}

	for (fd = 0; fd < 3; fd++) {
		if (is_a_console(fd)) {
			rd_dbg("using %d", fd);
			return fd;
		}
	}

	/* total failure */
	rd_err("couldn't get a file descriptor referring to the console");

	return -1;
}
