// SPDX-License-Identifier: GPL-3.0-or-later

#include "config.h"

#include <sys/uio.h>

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "rd/logging.h"

int log_priority = LOG_INFO;
int log_fd = STDERR_FILENO;
const char *log_progname = NULL;
size_t log_progname_len = 0;

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
	if (progname) {
		log_progname = progname;
		log_progname_len = strlen(progname);
	}
}

void rd_logging_close(void)
{
}

enum {
	LOG_FMT_DATETIME,
	LOG_FMT_DELIM_1,
	LOG_FMT_PROGNAME,
	LOG_FMT_DELIM_2,
	LOG_FMT_MESSAGE,
	LOG_FMT_EOL,
	LOG_FMT_MAX,
};

void rd_vmessage(const char *fmt, va_list ap)
{
	char stack_date[38]; // "[1970-01-01 00:00:00]"
	char stack_msg[80];
	struct iovec iov[LOG_FMT_MAX];

	memset(iov, 0, sizeof(iov));

	int used;
	struct tm t;
	time_t now = time(NULL);

	if (localtime_r(&now, &t) == &t) {
		used = snprintf(stack_date, sizeof(stack_date),
		                "[%04d-%02d-%02d %02d:%02d:%02d]",
		                t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
		                t.tm_hour, t.tm_min, t.tm_sec);

		iov[LOG_FMT_DATETIME].iov_base = stack_date;
		iov[LOG_FMT_DATETIME].iov_len = (size_t) used;

		iov[LOG_FMT_DELIM_1].iov_base = (char *) " ";
		iov[LOG_FMT_DELIM_1].iov_len = 1;
	}

	if (log_progname) {
		iov[LOG_FMT_PROGNAME].iov_base = (char *) log_progname;
		iov[LOG_FMT_PROGNAME].iov_len = log_progname_len;

		iov[LOG_FMT_DELIM_2].iov_base = (char *) ": ";
		iov[LOG_FMT_DELIM_2].iov_len = 2;
	}

	iov[LOG_FMT_EOL].iov_base = (char *) "\n";
	iov[LOG_FMT_EOL].iov_len = 1;

	va_list ap2;
	size_t size;
	char *buf = NULL;

	va_copy(ap2, ap);
	used = vsnprintf(NULL, 0, fmt, ap2);
	va_end(ap2);

	if (used >= (int) sizeof(stack_msg)) {
		size = (size_t) used + 1;
		buf = malloc(size);
	}

	if (!buf) {
		size = sizeof(stack_msg);
		buf = stack_msg;
	}

	va_copy(ap2, ap);
	used = vsnprintf(buf, size, fmt, ap2);
	va_end(ap2);

	iov[LOG_FMT_MESSAGE].iov_base = buf;
	iov[LOG_FMT_MESSAGE].iov_len = (size_t) used;

	fsync(log_fd);
	TEMP_FAILURE_RETRY(writev(log_fd, iov, LOG_FMT_MAX));

	if (buf != stack_msg)
		free(buf);

	return;
}

void rd_log_vmessage(int priority, const char *fmt, va_list ap)
{
	if (priority <= log_priority)
		rd_vmessage(fmt, ap);
}

void rd_log_message(int priority, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	if (priority <= log_priority) {
		rd_vmessage(fmt, ap);
	}
	va_end(ap);
}

void rd_log_setup_stderr(const char *logfile)
{
	if (getenv("RDLOG_STDERR"))
		return;

	char *rdlog = getenv("RDLOG");

	if (rdlog && !strcasecmp(rdlog, "console"))
		logfile = "/dev/console";

	if (!freopen(logfile, "w+", stderr))
		rd_fatal("freopen(%s): %m", logfile);

	setvbuf(stderr, NULL, _IONBF, 0);
}
