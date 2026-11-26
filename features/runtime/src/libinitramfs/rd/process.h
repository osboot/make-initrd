// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef __RD_PROCESS_H__
#define __RD_PROCESS_H__

#include <sys/types.h>

#include "compiler.h"

pid_t rd_waitpid_retry(pid_t pid, int *wstatus, int options) _access_(write_only, 2);

#endif /* __RD_PROCESS_H__ */
