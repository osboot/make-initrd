// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef __UEVENTD_H__
#define __UEVENTD_H__

#include <sys/types.h>
#include <stdint.h>
#include <unistd.h>
#include <dirent.h>

#include "compiler.h"
#include "cleanup.h"
#include "rd/path.h"
#include "rd/process.h"

#define F_ROOT_DIR  (1 << 0)
#define F_QUEUE_DIR (1 << 1)
#define F_PAUSE_DIR (1 << 2)
#define F_DIRTY     (1 << 3)
#define F_PAUSED    (1 << 4)
#define F_TIMER_ROOT (1 << 5)
#define F_TIMER_DIR  (1 << 6)

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
extern char *timer_dir;
extern char *uevent_dir;
extern char *handler_file;
extern uint64_t session;

/* queue-processor.c */
extern void process_events(struct watch *queue) _nonnull_(1) _noreturn_;

/* sources.c */
extern void sources_start(const char *dir_path);
extern int sources_handle_exit(pid_t pid, int wstatus);
extern int sources_next_timeout_ms(void);
extern void sources_tick(void);
extern void sources_stop_all(void);

#endif /* __UEVENTD_H__ */
