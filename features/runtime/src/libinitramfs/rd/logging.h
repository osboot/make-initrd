// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef __RD_LOGGING_H__
#define __RD_LOGGING_H__

#include <unistd.h>
#include <syslog.h>
#include <stdlib.h>
#include <stdarg.h>

#include "compiler.h"

_public_ void rd_logging_init(int log_fd, int level, const char *progname);
_public_ void rd_logging_close(void);
_public_ int rd_logging_level(const char *lvl)                           _nonnull_(1);
_public_ void rd_vmessage(const char *fmt, va_list ap)                   _printf_(1, 0);
_public_ void rd_log_vmessage(int priority, const char *fmt, va_list ap) _printf_(2, 0);
_public_ void rd_log_message(int priority, const char *fmt, ...)         _printf_(2, 3);
_public_ void rd_log_setup_stderr(const char *logfile)                   _nonnull_(1);
_public_ const char *rd_log_resolve_file(const char *env_var,
                                         const char *fallback)           _nonnull_(2);

#define rd_fatal(format, arg...)                    \
	do {                                        \
		rd_log_message(LOG_CRIT, "%s:%d: " format, __FILE__, __LINE__, ##arg); \
		_exit(EXIT_FAILURE);                \
	} while (0)

#define rd_err(format, arg...)  rd_log_message(LOG_ERR,   format, ##arg)
#define rd_info(format, arg...) rd_log_message(LOG_INFO,  format, ##arg)
#define rd_dbg(format, arg...)  rd_log_message(LOG_DEBUG, format, ##arg)

#endif /* __RD_LOGGING_H__ */
