/* SPDX-License-Identifier: GPL-2.0-only */

#include <linux/bpf.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#include <bpf/bpf_helpers.h>
#include <bpf/bpf_core_read.h>
#include <bpf/bpf_tracing.h>

#include "procacct.h"

char _license[] SEC("license") = "GPL";

struct mm_struct {
	unsigned long arg_start, arg_end, env_start, env_end;
} __attribute__((preserve_access_index));

struct task_struct {
	pid_t pid;
	struct mm_struct *mm;
} __attribute__((preserve_access_index));

struct {
	__uint(type, BPF_MAP_TYPE_RINGBUF);
	__uint(max_entries, 256 * sizeof(struct task_cmdline));
} ringbuf SEC(".maps");

SEC("raw_tracepoint/sched_process_exec")
int procacct_process_exec(struct bpf_raw_tracepoint_args *ctx __attribute__((unused)))
{
	struct task_cmdline *buf;

	struct task_struct *task = (void *) bpf_get_current_task_btf();
	char *arg_start = (char *) BPF_CORE_READ(task, mm, arg_start);
	char *arg_end   = (char *) BPF_CORE_READ(task, mm, arg_end);
	size_t len = arg_end - arg_start;

	buf = bpf_ringbuf_reserve(&ringbuf, sizeof(*buf), 0);
	if (!buf)
		return 0;

	if (len >= sizeof(buf->cmdline))
		len = sizeof(buf->cmdline);

	if (bpf_probe_read_user(buf->cmdline, len, arg_start)) {
		bpf_ringbuf_discard(buf, 0);
		return 0;
	}

	buf->pid = BPF_CORE_READ(task, pid);
	buf->cmdline_len = len;

	bpf_ringbuf_submit(buf, 0);
	return 0;
}
