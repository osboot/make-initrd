// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef __RD_UEVENT_H__
#define __RD_UEVENT_H__

#include <stdint.h>

#include "compiler.h"

struct rd_uevent_queue {
	const char *filter_dir;
	const char *name;
	int queue_dirfd;
	int temp_dirfd;
	uint64_t seq;
};

char *rd_getenv_or_die(const char *name) __nonnull(1);

void rd_uevent_queue_open(struct rd_uevent_queue *q,
                          const char *filter_dir,
                          const char *queue_name) __nonnull(1, 2, 3);

void rd_uevent_queue_close(struct rd_uevent_queue *q) __nonnull(1);

void rd_uevent_emit(struct rd_uevent_queue *q,
                    const char *event_name,
                    const char *fmt, ...) __nonnull(1, 2, 3) __printf(3, 4);

#endif /* __RD_UEVENT_H__ */
