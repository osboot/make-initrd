// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2022  Arseny Maslennikov <arseny@altlinux.org>
 * All rights reserved.
 */

#include "config.h"

#include <sys/types.h>
#include <sys/wait.h>

#include <unistd.h>
#include <errno.h>

#include "temp_failure_retry.h"
#include "ueventd.h"

pid_t waitpid_retry(pid_t pid, int *wstatus, int options)
{
	return (pid_t) TEMP_FAILURE_RETRY(waitpid(pid, wstatus, options));
}
