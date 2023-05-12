/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef __UEVENTD_H__
#define __UEVENTD_H__

#include <sys/types.h>
#include <stdint.h>

#define F_ROOT_DIR  (1 << 0)
#define F_QUEUE_DIR (1 << 1)
#define F_PAUSE_DIR (1 << 2)
#define F_DIRTY     (1 << 3)
#define F_PAUSED    (1 << 4)

struct watch {
	struct watch *next;
	int   q_flags;
	int   q_watchfd;
	int   q_parentfd;
	ino_t q_ino;
	pid_t q_pid;
	char  q_name[];
};

extern char *filter_dir;
extern char *uevent_dir;
extern char *handler_file;
extern uint64_t session;

/* queue-processor.c */
extern void process_events(struct watch *queue) __attribute__((nonnull(1), noreturn));

/* path.c */
#include <dirent.h>

extern DIR *xopendir(const char *path)                              __attribute__((nonnull(1)));
extern struct dirent *xreaddir(DIR *d, const char *path)            __attribute__((nonnull(1, 2)));
extern int empty_directory(const char *path)                        __attribute__((nonnull(1)));
extern ssize_t read_retry(int fd, void *buf, size_t count)          __attribute__((nonnull(2)));
extern ssize_t write_retry(int fd, const void *buf, size_t count)   __attribute__((nonnull(2)));
extern ssize_t write_loop(int fd, const char *buffer, size_t count) __attribute__((nonnull(2)));

/* process.c */
extern pid_t waitpid_retry(pid_t pid, int *wstatus, int options);

#include <dirent.h>

extern  int is_dot_dir(struct dirent *ent) __attribute__((nonnull(1)));

/* logging.c */
#include <unistd.h>
#include <syslog.h>
#include <stdlib.h>
#include <stdarg.h>

extern void rd_logging_init(int log_fd, int level, const char *progname);
extern void rd_logging_close(void);
extern int rd_logging_level(const char *lvl)                           __attribute__((nonnull(1)));
extern void rd_vmessage(const char *fmt, va_list ap)                   __attribute__((format(printf, 1, 0)));
extern void rd_log_vmessage(int priority, const char *fmt, va_list ap) __attribute__((format(printf, 2, 0)));
extern void rd_log_message(int priority, const char *fmt, ...)         __attribute__((format(printf, 2, 3)));

#define rd_fatal(format, arg...)                    \
	do {                                        \
		rd_log_message(LOG_CRIT, "%s:%d: " format, __FILE__, __LINE__, ##arg); \
		_exit(EXIT_FAILURE);                \
	} while (0)

#define rd_err(format, arg...)  rd_log_message(LOG_ERR,   format, ##arg)
#define rd_info(format, arg...) rd_log_message(LOG_INFO,  format, ##arg)
#define rd_dbg(format, arg...)  rd_log_message(LOG_DEBUG, format, ##arg)

#endif /* __UEVENTD_H__ */
