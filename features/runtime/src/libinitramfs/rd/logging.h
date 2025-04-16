// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef __RD_LOGGING_H__
#define __RD_LOGGING_H__

#include <unistd.h>
#include <syslog.h>
#include <stdlib.h>
#include <stdarg.h>

void rd_logging_init(int log_fd, int level, const char *progname);
void rd_logging_close(void);
int rd_logging_level(const char *lvl)                           __attribute__((nonnull(1)));
void rd_vmessage(const char *fmt, va_list ap)                   __attribute__((format(printf, 1, 0)));
void rd_log_vmessage(int priority, const char *fmt, va_list ap) __attribute__((format(printf, 2, 0)));
void rd_log_message(int priority, const char *fmt, ...)         __attribute__((format(printf, 2, 3)));
void rd_log_setup_stderr(const char *logfile)                   __attribute__((nonnull(1)));

#define rd_fatal(format, arg...)                    \
	do {                                        \
		rd_log_message(LOG_CRIT, "%s:%d: " format, __FILE__, __LINE__, ##arg); \
		_exit(EXIT_FAILURE);                \
	} while (0)

#define rd_err(format, arg...)  rd_log_message(LOG_ERR,   format, ##arg)
#define rd_info(format, arg...) rd_log_message(LOG_INFO,  format, ##arg)
#define rd_dbg(format, arg...)  rd_log_message(LOG_DEBUG, format, ##arg)

#endif /* __RD_LOGGING_H__ */
