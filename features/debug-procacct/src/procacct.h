// SPDX-License-Identifier: GPL-2.0-only
#ifndef __PROCACCT_H__
#define __PROCACCT_H__

#define MAX_ARGS_LEN 4096

struct task_cmdline {
	pid_t pid;
	size_t cmdline_len;
	char cmdline[MAX_ARGS_LEN];
};

#endif /* __PROCACCT_H__ */
