// SPDX-License-Identifier: GPL-2.0
/* procacct.c
 *
 * Demonstrator of fetching resource data on task exit, as a way
 * to accumulate accurate program resource usage statistics, without
 * prior identification of the programs. For that, the fields for
 * device and inode of the program executable binary file are also
 * extracted in addition to the command string.
 *
 * The TGID together with the PID and the AGROUP flag allow
 * identification of threads in a process and single-threaded processes.
 * The ac_tgetime field gives proper whole-process walltime.
 *
 * Written (changed) by Thomas Orgis, University of Hamburg in 2022
 *
 * This is a cheap derivation (inheriting the style) of getdelays.c:
 *
 * Utility to get per-pid and per-tgid delay accounting statistics
 * Also illustrates usage of the taskstats interface
 *
 * Copyright (C) Shailabh Nagar, IBM Corp. 2005
 * Copyright (C) Balbir Singh, IBM Corp. 2006
 * Copyright (c) Jay Lan, SGI. 2006
 */
#include <linux/genetlink.h>
#include <linux/acct.h>
#include <linux/taskstats.h>
#include <linux/kdev_t.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/epoll.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <search.h>

#include "rd/logging.h"
#include "rd/memory.h"

/* Maximum size of response requested or message sent */
#define MAX_MSG_SIZE 2048

/* Maximum number of cpus expected to be specified in a cpumask */
#define MAX_CPUS 64

/*
 * Generic macros for dealing with netlink sockets. Might be duplicated
 * elsewhere. It is recommended that commercial grade applications use
 * libnl or libnetlink and use the interfaces provided by the library
 */
#define GENLMSG_DATA(glh)	((void *)(NLMSG_DATA(glh) + GENL_HDRLEN))
#define GENLMSG_PAYLOAD(glh)	(NLMSG_PAYLOAD(glh, 0) - GENL_HDRLEN)
#define NLA_DATA(na)		((void *)((char *)(na) + NLA_HDRLEN))
#define NLA_PAYLOAD(len)	(len - NLA_HDRLEN)

static pid_t current_pid;
static int rcvbufsz = MAX_MSG_SIZE * 32;
static char name[100];
static int fd_out = 1;

struct msgtemplate {
	struct nlmsghdr n;
	struct genlmsghdr g;
	char buf[MAX_MSG_SIZE];
};

static void *proc_root = NULL;

struct proc_cmdline {
	pid_t pid;
	char *cmdline;
};

struct ctx_netlink {
	uint16_t cpuid;
	uint16_t cpumask_len;
	char cpumask[100 + 6 * MAX_CPUS];
};

enum {
	FD_NETLINK,
	FD_MAX,
};

#define EV_MAX (FD_MAX * 32)

struct fd_handler;
typedef int (*fdhandler_t)(struct fd_handler *);

struct fd_handler {
	int fd;
	fdhandler_t fd_prepare;
	fdhandler_t fd_handler;
	fdhandler_t fd_finish;
	void *data;
};

static struct fd_handler fd_handler_list[FD_MAX];

static void usage(void)                                                       __attribute__((noreturn));
static int proc_compare(const void *a, const void *b)                         __attribute__((nonnull(1, 2)));
static int process_netlink_events(struct fd_handler *el);
static int prepare_netlink(struct fd_handler *el);
static int finish_netlink(struct fd_handler *el);
static void setup_netlink_fd(struct fd_handler *el);
static ssize_t send_cmd(int fd, uint16_t nlmsg_type, uint8_t genl_cmd,
                        uint16_t nla_type, void *nla_data, uint16_t nla_len);
static uint16_t get_family_id(int fd);
static void print_procacct(int fd, struct taskstats *t)                       __attribute__((nonnull(2)));
static void handle_aggr(struct nlattr *na, int fd)                            __attribute__((nonnull(1)));
static int setup_epoll_fd(struct fd_handler list[FD_MAX]);

int proc_compare(const void *a, const void *b)
{
	pid_t pid_a = ((struct proc_cmdline *)a)->pid;
	pid_t pid_b = ((struct proc_cmdline *)b)->pid;

	if (pid_a < pid_b)
		return -1;
	if (pid_a > pid_b)
		return 1;
	return 0;
}

void usage(void)
{
	fprintf(stderr, "procacct [-o logfile] [-r bufsize]\n");
	exit(1);
}

/*
 * Create a raw netlink socket and bind
 */
void setup_netlink_fd(struct fd_handler *el)
{
	struct sockaddr_nl local;

	el->fd = socket(AF_NETLINK, SOCK_RAW | SOCK_NONBLOCK | SOCK_CLOEXEC, NETLINK_GENERIC);
	if (el->fd < 0)
		rd_fatal("error creating Netlink socket: %m");

	if (rcvbufsz && setsockopt(el->fd, SOL_SOCKET, SO_RCVBUF, &rcvbufsz, sizeof(rcvbufsz)) < 0)
		rd_fatal("unable to set socket rcv buf size to %d", rcvbufsz);

	memset(&local, 0, sizeof(local));
	local.nl_family = AF_NETLINK;

	if (bind(el->fd, (struct sockaddr *) &local, sizeof(local)) < 0)
		rd_fatal("unable bind to socket");

	el->fd_handler = process_netlink_events;
	el->fd_prepare = prepare_netlink;
	el->fd_finish  = finish_netlink;
}

ssize_t send_cmd(int fd, uint16_t nlmsg_type, uint8_t genl_cmd, uint16_t nla_type, void *nla_data, uint16_t nla_len)
{
	struct nlattr *na;
	struct sockaddr_nl nladdr;

	struct msgtemplate msg;

	msg.n.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
	msg.n.nlmsg_type = nlmsg_type;
	msg.n.nlmsg_flags = NLM_F_REQUEST;
	msg.n.nlmsg_seq = 0;
	msg.n.nlmsg_pid = (uint32_t) current_pid;

	msg.g.cmd = genl_cmd;
	msg.g.version = 0x1;

	na = (struct nlattr *) GENLMSG_DATA(&msg);
	na->nla_type = nla_type;
	na->nla_len = nla_len + 1 + NLA_HDRLEN;

	memcpy(NLA_DATA(na), nla_data, nla_len);

	msg.n.nlmsg_len += NLMSG_ALIGN(na->nla_len);

	char *buf = (char *) &msg;
	ssize_t buflen = msg.n.nlmsg_len;

	memset(&nladdr, 0, sizeof(nladdr));
	nladdr.nl_family = AF_NETLINK;

	ssize_t ret;

	while ((ret = sendto(fd, buf, (size_t) buflen, 0, (struct sockaddr *) &nladdr, sizeof(nladdr))) < buflen) {
		if (ret > 0) {
			buf += ret;
			buflen -= ret;
		} else if (errno != EINTR && errno != EAGAIN) {
			return -1;
		}
	}

	return 0;
}


/*
 * Probe the controller in genetlink to find the family id
 * for the TASKSTATS family
 */
uint16_t get_family_id(int fd)
{
	struct msgtemplate msg;

	ssize_t ret = send_cmd(fd, GENL_ID_CTRL, CTRL_CMD_GETFAMILY,
	                       CTRL_ATTR_FAMILY_NAME,
	                       (char *) TASKSTATS_GENL_NAME, sizeof(TASKSTATS_GENL_NAME));
	if (ret < 0)
		return 0;

	ret = recv(fd, &msg, sizeof(msg), 0);
	if (ret < 0)
		rd_fatal("receive NETLINK family: %m");

	if (msg.n.nlmsg_type == NLMSG_ERROR || !NLMSG_OK((&msg.n), ret)) {
		struct nlmsgerr *err = NLMSG_DATA(&msg);
		rd_fatal("receive NETLINK family: %s (errno=%d)", strerror(-err->error),  err->error);
	}

	struct nlattr *na;

	na = (struct nlattr *) GENLMSG_DATA(&msg);
	na = (struct nlattr *) ((char *) na + NLA_ALIGN(na->nla_len));

	if (na->nla_type != CTRL_ATTR_FAMILY_ID)
		rd_fatal("unexpected family id");

	return *(uint16_t *) NLA_DATA(na);
}

void print_procacct(int fd, struct taskstats *t)
{
	struct proc_cmdline key = { .pid = (pid_t) t->ac_pid };
	struct proc_cmdline *proc = tfind(&key, proc_root, proc_compare);

	dprintf(fd,
	        "%c\t%u\t%u\t%u\t%llu\t%llu\t%llu\t%llu\t%llu\t%llu\t%llu\t%llu\t%llu\t[%s]\t%s\n",
	        // First letter: T is a mere thread, G the last in a group, U  unknown.
	        (t->version >= 12 ? (t->ac_flag & AGROUP ? 'P' : 'T') : '?'),
	        (t->ac_pid),                         // pid
	        (t->version >= 12 ? t->ac_tgid : 0), // tgid
	        (t->ac_ppid),                        // ppid
	        (t->ac_btime64),                     // btime
	        (t->ac_etime),                       // wall
	        (t->ac_utime + t->ac_stime),         // cputime
	        (t->hiwater_vm),                     // vmusage
	        (t->hiwater_rss),                    // rssusage
	        (t->read_char),                      // bytes read
	        (t->write_char),                     // bytes write
	        (t->read_bytes),                     // bytes of read I/O
	        (t->write_bytes),                    // bytes of write I/O
	        (t->ac_comm),                        // comm
	        (proc ? proc->cmdline : "")          // cmdline
	       );
}

void handle_aggr(struct nlattr *na, int fd)
{
	int nla_type = na->nla_type;
	int aggr_len = NLA_PAYLOAD(na->nla_len);
	int len2 = 0;

	na = (struct nlattr *) NLA_DATA(na);
	while (len2 < aggr_len) {
		switch (na->nla_type) {
			case TASKSTATS_TYPE_PID:
			case TASKSTATS_TYPE_TGID:
				break;
			case TASKSTATS_TYPE_STATS:
				if (nla_type == TASKSTATS_TYPE_AGGR_PID)
					print_procacct(fd, (struct taskstats *) NLA_DATA(na));
				break;
			case TASKSTATS_TYPE_NULL:
				break;
			default:
				rd_err("unknown nested nla_type %d", na->nla_type);
				break;
		}
		len2 += NLA_ALIGN(na->nla_len);
		na = (struct nlattr *)((char *) na + NLA_ALIGN(na->nla_len));
	}
}

int prepare_netlink(struct fd_handler *el)
{
	struct ctx_netlink *ctx;
	ssize_t ret;

	ctx = rd_calloc_or_die(1, sizeof(*ctx));

	long np = sysconf(_SC_NPROCESSORS_ONLN);
	if (np > 1)
		snprintf(ctx->cpumask, sizeof(ctx->cpumask), "0-%ld", np - 1);
	else
		snprintf(ctx->cpumask, sizeof(ctx->cpumask), "1");

	if ((strlen(ctx->cpumask) + 1) > USHRT_MAX) {
		rd_err("cpumask too long");
		free(ctx);
		return -1;
	}

	ctx->cpumask_len = (uint16_t) strlen(ctx->cpumask) + 1;

	ctx->cpuid = get_family_id(el->fd);
	if (!ctx->cpuid) {
		rd_err("error getting family id, errno=%d", errno);
		free(ctx);
		return -1;
	}

	ret = send_cmd(el->fd, ctx->cpuid, TASKSTATS_CMD_GET,
	               TASKSTATS_CMD_ATTR_REGISTER_CPUMASK,
		       &ctx->cpumask, ctx->cpumask_len);
	if (ret < 0) {
		rd_err("error sending register cpumask");
		free(ctx);
		return -1;
	}

	el->data = ctx;

	return 0;
}

int finish_netlink(struct fd_handler *el)
{
	struct ctx_netlink *ctx = el->data;
	ssize_t ret;

	ret = send_cmd(el->fd, ctx->cpuid, TASKSTATS_CMD_GET,
	               TASKSTATS_CMD_ATTR_DEREGISTER_CPUMASK,
		       &ctx->cpumask, ctx->cpumask_len);
	if (ret < 0) {
		rd_err("error sending deregister cpumask");
		return -1;
	}
	free(ctx);
	return 0;
}

int process_netlink_events(struct fd_handler *el)
{
	ssize_t ret;

	while (1) {
		struct msgtemplate msg;
		struct nlattr *na;

		errno = 0;
		ret = recv(el->fd, &msg, sizeof(msg), 0);
		if (ret < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				return 0;
			if (errno == EINTR)
				continue;
			rd_fatal("nonfatal reply error: %m");
		}

		if (msg.n.nlmsg_type == NLMSG_ERROR || !NLMSG_OK((&msg.n), ret)) {
			struct nlmsgerr *err = NLMSG_DATA(&msg);
			rd_fatal("reply error: %s (errno=%d)", strerror(-err->error), err->error);
		}

		ret = GENLMSG_PAYLOAD(&msg.n);
		na = (struct nlattr *) GENLMSG_DATA(&msg);

		ssize_t len = 0;

		while (len < ret) {
			switch (na->nla_type) {
				case TASKSTATS_TYPE_NULL:
					break;
				case TASKSTATS_TYPE_AGGR_PID:
				case TASKSTATS_TYPE_AGGR_TGID:
					/* For nested attributes, na follows */
					handle_aggr(na, fd_out);
					break;
				default:
					rd_err("unexpected nla_type %d", na->nla_type);
			}

			len += NLA_ALIGN(na->nla_len);
			na = (struct nlattr *) (GENLMSG_DATA(&msg) + len);
		}
	}

	return 0;
}

int setup_epoll_fd(struct fd_handler list[FD_MAX])
{
	int epollfd;
	struct epoll_event ev = { .events = EPOLLIN };

	if ((epollfd = epoll_create1(EPOLL_CLOEXEC)) < 0)
		rd_fatal("epoll_create1: %m");

	for (int i = 0; i < FD_MAX; i++) {
		ev.data.ptr = &list[i];

		if (epoll_ctl(epollfd, EPOLL_CTL_ADD, list[i].fd, &ev) < 0)
			rd_fatal("epoll_ctl: %m");
	}

	return epollfd;
}

int main(int argc, char *argv[])
{
	int fd_epoll = -1;
	int write_file = 0;
	char *logfile = NULL;

	while (1) {
		int c = getopt(argc, argv, "o:r:");
		if (c < 0)
			break;

		switch (c) {
			case 'o':
				logfile = strdup(optarg);
				write_file = 1;
				break;
			case 'r':
				rcvbufsz = atoi(optarg);
				if (rcvbufsz < 0)
					rd_fatal("invalid rcv buf size");
				break;
			default:
				usage();
		}
	}

	if (write_file) {
		fd_out = open(logfile, O_WRONLY | O_CREAT | O_TRUNC | O_SYNC, 0644);
		if (fd_out < 0)
			rd_fatal("cannot open output file: %s: %m", logfile);
	}

	current_pid = getpid();

	setup_netlink_fd(&fd_handler_list[FD_NETLINK]);
	fd_epoll = setup_epoll_fd(fd_handler_list);

	for (int i = 0; i < FD_MAX; i++) {
		if (fd_handler_list[i].fd < 0 || !fd_handler_list[i].fd_prepare)
			continue;
		if (fd_handler_list[i].fd_prepare(&fd_handler_list[i]) < 0)
			rd_fatal("unable to prepare file descriptor");
	}

	while (1) {
		struct epoll_event ev[EV_MAX];
		int fdcount;

		errno = 0;
		fdcount = epoll_wait(fd_epoll, ev, EV_MAX, -1);

		if (fdcount < 0) {
			if (errno == EINTR)
				continue;
			rd_fatal("epoll_wait: %m");
		}

		for (int i = 0; i < fdcount; i++) {
			if (!(ev[i].events & EPOLLIN))
				continue;

			struct fd_handler *fde = ev[i].data.ptr;

			if (fde->fd_handler(fde) != 0)
				goto err;
		}
	}

err:
	for (int i = 0; i < FD_MAX; i++) {
		if (fd_handler_list[i].fd < 0)
			continue;

		if (epoll_ctl(fd_epoll, EPOLL_CTL_DEL, fd_handler_list[i].fd, NULL) < 0)
			rd_err("epoll_ctl: %m");

		if (fd_handler_list[i].fd_finish)
			fd_handler_list[i].fd_finish(&fd_handler_list[i]);

		close(fd_handler_list[i].fd);
	}
	close(fd_epoll);

	if (fd_out)
		close(fd_out);

	return 0;
}
