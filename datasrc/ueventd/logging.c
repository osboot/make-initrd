/* SPDX-License-Identifier: GPL-2.0-or-later */

#include <stdio.h>
#include <stdarg.h>
#include <strings.h>
#include <errno.h>
#include <time.h>

#include "ueventd.h"

int log_priority = LOG_INFO;
int log_fd = STDERR_FILENO;
const char *log_progname = NULL;

int rd_logging_level(const char *name)
{
	if (!strcasecmp(name, "debug"))   return LOG_DEBUG;
	if (!strcasecmp(name, "info"))    return LOG_INFO;
	if (!strcasecmp(name, "warning")) return LOG_WARNING;
	if (!strcasecmp(name, "error"))   return LOG_ERR;
	return log_priority;
}

void rd_logging_init(int fd, int loglevel, const char *progname)
{
	log_priority = loglevel;
	log_fd = fd;
	log_progname = progname;
}

void rd_logging_close(void)
{
}

void rd_message(int priority, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	if (priority <= log_priority) {
		time_t ts = time(NULL);
		struct tm *t = localtime(&ts);
		dprintf(log_fd, "[%04d-%02d-%02d %02d:%02d:%02d] ",
		        t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
		        t->tm_hour, t->tm_min, t->tm_sec);
		if (log_progname)
			dprintf(log_fd, "%s: ", log_progname);
		vdprintf(log_fd, fmt, ap);
		dprintf(log_fd, "\n");
	}
	va_end(ap);
}
