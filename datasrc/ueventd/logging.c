/* SPDX-License-Identifier: GPL-2.0-or-later */

#include <stdio.h>
#include <stdarg.h>
#include <strings.h>
#include <errno.h>
#include <time.h>

#include "ueventd.h"

#define default_logfile "/var/log/ueventd.log"

int log_priority = LOG_INFO;

int rd_logging_level(const char *name)
{
	if (!strcasecmp(name, "debug"))   return LOG_DEBUG;
	if (!strcasecmp(name, "info"))    return LOG_INFO;
	if (!strcasecmp(name, "warning")) return LOG_WARNING;
	if (!strcasecmp(name, "error"))   return LOG_ERR;
	return log_priority;
}

void rd_logging_init(int loglevel)
{
	if (!getenv("UEVENTD_USE_STDERR")) {
		char *rdlog = getenv("RDLOG");
		const char *logfile = default_logfile;

		if (rdlog && !strcasecmp(rdlog, "console"))
			logfile = "/dev/console";

		FILE *cons = fopen(logfile, "w+");
		if (!cons)
			rd_fatal("open(%s): %m", logfile);

		fclose(stderr);
		stderr = cons;
	}

	log_priority = loglevel;
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
		fprintf(stderr, "[%04d-%02d-%02d %02d:%02d:%02d] %s: ",
		        t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
		        t->tm_hour, t->tm_min, t->tm_sec,
		        program_invocation_short_name);
		vfprintf(stderr, fmt, ap);
		fprintf(stderr, "\n");
	}
	va_end(ap);
}
