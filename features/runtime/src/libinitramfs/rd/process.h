// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RD_PROCESS_H
#define RD_PROCESS_H

#include <sys/types.h>

#include "compiler.h"

_public_ pid_t rd_waitpid_retry(pid_t pid, int *wstatus, int options) _access_(write_only, 2);

#endif /* RD_PROCESS_H */
