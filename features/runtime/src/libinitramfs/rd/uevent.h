// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RD_UEVENT_H
#define RD_UEVENT_H

#include <stdint.h>

#include "compiler.h"

struct rd_uevent_queue {
	const char *filter_dir;
	const char *name;
	int queue_dirfd;
	int temp_dirfd;
	uint64_t seq;
};

_public_ char *rd_getenv_or_die(const char *name) _nonnull_(1);

_public_ void rd_uevent_queue_open(struct rd_uevent_queue *q,
                                   const char *filter_dir,
                                   const char *queue_name) _nonnull_(1, 2, 3);

_public_ void rd_uevent_queue_close(struct rd_uevent_queue *q) _nonnull_(1);

_public_ void rd_uevent_emit(struct rd_uevent_queue *q,
                             const char *event_name,
                             const char *fmt, ...) _nonnull_(1, 2, 3) _printf_(3, 4);

_public_ void rd_uevent_source_init(struct rd_uevent_queue *q,
                                    int argc, char **argv,
                                    const char *queue_name,
                                    const char *logfile_env,
                                    const char *default_logfile)
	_wronly_ptr_(1) _rdonly_ptr_(3) _rdonly_ptr_(4) _rdonly_ptr_(5) _rdonly_ptr_(6);

_public_ void rd_uevent_source_loop(int fd, short poll_events,
                                    void (*dispatch)(void *data),
                                    void *data) _nonnull_(3);

_public_ void rd_uevent_source_close(struct rd_uevent_queue *q) _rdwr_ptr_(1);

#endif /* RD_UEVENT_H */
