/*
 * utmp.c	Routines to read/write the utmp and wtmp files.
 *		Basically just wrappers around the library routines.
 *
 * Version:	@(#)utmp.c  2.77  09-Jun-1999  miquels@cistron.nl
 *
 * Copyright (C) 1999 Miquel van Smoorenburg.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 */

#include "config.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/utsname.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <utmp.h>

#include "reboot.h"

/*
 *	Log an event in the wtmp file (reboot, runlevel)
 */
void write_wtmp(
const char *user,		/* name of user */
const char *id,			/* inittab ID */
int pid,			/* PID of process */
short type,			/* TYPE of entry */
const char *line)		/* Which line is this */
{
	int fd;
	struct utmp utmp;
	struct utsname uname_buf;
	struct timespec ts;

	/*
	 *	Can't do much if WTMP_FILE is not present or not writable.
	 */
	if (access(WTMP_FILE, W_OK) < 0)
		return;

	/*
	 *	Try to open the wtmp file. Note that we even try
	 *	this if we have updwtmp() so we can see if the
	 *	wtmp file is accessible.
	 */
	if ((fd = open(WTMP_FILE, O_WRONLY|O_APPEND)) < 0) return;

	/*
	 *	Zero the fields and enter new fields.
	 */
	memset(&utmp, 0, sizeof(utmp));

	if (!clock_gettime(CLOCK_REALTIME, &ts)) {
		utmp.ut_tv.tv_sec = ts.tv_sec;
		utmp.ut_tv.tv_usec = (int) ts.tv_nsec / 1000;
	}

	utmp.ut_pid  = pid;
	utmp.ut_type = type;
	strncpy(utmp.ut_name, user, sizeof(utmp.ut_name));
	strncpy(utmp.ut_id  , id  , sizeof(utmp.ut_id  ));
	strncpy(utmp.ut_line, line, sizeof(utmp.ut_line));

        /* Put the OS version in place of the hostname */
        if (uname(&uname_buf) == 0)
		strncpy(utmp.ut_host, uname_buf.release, sizeof(utmp.ut_host));

#if HAVE_UPDWTMP
	updwtmp(WTMP_FILE, &utmp);
#else
	write(fd, (char *)&utmp, sizeof(utmp));
#endif
	close(fd);
}
