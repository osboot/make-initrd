#include <stdio.h>
#include <stdarg.h>
#include <strings.h>
#include <unistd.h>
#include <errno.h>
#include <error.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#include "uevent-logging.h"

extern int log_priority;
extern char *logfile;

int
logging_level(const char *name)
{
	if (!strcasecmp(name, "debug"))
		return LOG_DEBUG;

	if (!strcasecmp(name, "info"))
		return LOG_INFO;

	if (!strcasecmp(name, "warning"))
		return LOG_WARNING;

	if (!strcasecmp(name, "error"))
		return LOG_ERR;

	return 0;
}

void
logging_init(void)
{
	int fd;

	if (!logfile)
		return;

	if ((fd = open(logfile, O_WRONLY|O_APPEND|O_CREAT, S_IRUSR|S_IWUSR)) < 0)
		error(EXIT_FAILURE, errno, "%s: open", PROGRAM_NAME);

	close(STDOUT_FILENO);
	dup2(fd, STDOUT_FILENO);

	close(STDERR_FILENO);
	dup2(fd, STDERR_FILENO);

	close(fd);
}

void
logging_close(void)
{
	if (!logfile)
		return;

	close(STDOUT_FILENO);
	close(STDERR_FILENO);
}

void
__attribute__ ((format (printf, 2, 3)))
message(int priority, const char *fmt, ...)
{
	va_list ap;
	struct timespec tp = { 0 };

	if (priority > log_priority)
		return;

	clock_gettime(CLOCK_MONOTONIC, &tp);
	fprintf(stderr, "[%lld.%ld] ", (long long int) tp.tv_sec, tp.tv_nsec);

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	va_end(ap);

	fflush(stderr);
}
