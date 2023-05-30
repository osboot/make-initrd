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

/* Maximum size of response requested or message sent */
#define MAX_MSG_SIZE 1024

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
static int rcvbufsz;
static char name[100];

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

static void usage(void)                                                       __attribute__((noreturn));
static int proc_compare(const void *a, const void *b)                         __attribute__((nonnull(1, 2)));
static int create_nl_socket(int protocol);
static ssize_t send_cmd(int fd, uint16_t nlmsg_type, uint8_t genl_cmd,
                        uint16_t nla_type, void *nla_data, uint16_t nla_len);
static uint16_t get_family_id(int fd);
static void print_procacct(int fd, struct taskstats *t)                       __attribute__((nonnull(2)));
static void handle_aggr(struct nlattr *na, int fd)                            __attribute__((nonnull(1)));

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
	fprintf(stderr, "procacct [-o logfile] [-r bufsize] [-m cpumask]\n");
	exit(1);
}

/*
 * Create a raw netlink socket and bind
 */
int create_nl_socket(int protocol)
{
	int fd;
	struct sockaddr_nl local;

	fd = socket(AF_NETLINK, SOCK_RAW, protocol);
	if (fd < 0)
		return -1;

	if (rcvbufsz) {
		if (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &rcvbufsz, sizeof(rcvbufsz)) < 0) {
			rd_err("unable to set socket rcv buf size to %d", rcvbufsz);
			goto error;
		}
	}

	memset(&local, 0, sizeof(local));
	local.nl_family = AF_NETLINK;

	if (bind(fd, (struct sockaddr *) &local, sizeof(local)) < 0) {
		rd_err("unable bind to socket");
		goto error;
	}

	return fd;
error:
	close(fd);
	return -1;
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
	struct {
		struct nlmsghdr n;
		struct genlmsghdr g;
		char buf[256];
	} ans;

	strcpy(name, TASKSTATS_GENL_NAME);

	ssize_t ret = send_cmd(fd, GENL_ID_CTRL, CTRL_CMD_GETFAMILY,
	                       CTRL_ATTR_FAMILY_NAME, name, strlen(TASKSTATS_GENL_NAME) + 1);
	if (ret < 0)
		return 0;

	ret = recv(fd, &ans, sizeof(ans), 0);

	if (ans.n.nlmsg_type == NLMSG_ERROR || (ret < 0) || !NLMSG_OK((&ans.n), ret))
		return 0;

	struct nlattr *na;

	na = (struct nlattr *) GENLMSG_DATA(&ans);
	na = (struct nlattr *) ((char *) na + NLA_ALIGN(na->nla_len));

	uint16_t id = 0;

	if (na->nla_type == CTRL_ATTR_FAMILY_ID)
		id = *(uint16_t *) NLA_DATA(na);

	return id;
}

void print_procacct(int fd, struct taskstats *t)
{
	struct proc_cmdline key = { .pid = (pid_t) t->ac_pid };
	struct proc_cmdline *proc = tfind(&key, proc_root, proc_compare);

	dprintf(fd,
	        "%c\t%u\t%u\t%llu\t%llu\t%llu\t%llu\t %llu\t[%s]\t%s\n",
	        // First letter: T is a mere thread, G the last in a group, U  unknown.
	        (t->version >= 12 ? (t->ac_flag & AGROUP ? 'P' : 'T') : '?'),
	        (t->ac_pid),                         // pid
	        (t->version >= 12 ? t->ac_tgid : 0), // tgid
	        (t->ac_btime64),                     // btime
	        (t->ac_etime),                       // wall
	        (t->ac_utime + t->ac_stime),         // cputime
	        (t->hiwater_vm),                     // vmusage
	        (t->hiwater_rss),                    // rssusage
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

int main(int argc, char *argv[])
{
	ssize_t ret;
	uint16_t id;

	char cpumask[100 + 6 * MAX_CPUS];
	uint16_t cpumask_len;

	int fd_nlink = -1;
	int fd_out = 1;
	int write_file = 0;
	int maskset = 0;
	char *logfile = NULL;

	while (1) {
		int c = getopt(argc, argv, "m:o:r:");
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
			case 'm':
				strlcpy(cpumask, optarg, sizeof(cpumask));
				maskset = 1;
				break;
			default:
				usage();
		}
	}
	if (!maskset) {
		long np = sysconf(_SC_NPROCESSORS_ONLN);
		if (np > 1)
			snprintf(cpumask, sizeof(cpumask), "0-%ld", np - 1);
		else
			snprintf(cpumask, sizeof(cpumask), "1");
	}

	if ((strlen(cpumask) + 1) > USHRT_MAX)
		rd_fatal("cpumask too long");

	cpumask_len = (uint16_t) strlen(cpumask) + 1;

	if (write_file) {
		fd_out = open(logfile, O_WRONLY | O_CREAT | O_TRUNC | O_SYNC, 0644);
		if (fd_out < 0)
			rd_fatal("cannot open output file: %s: %m", logfile);
	}

	fd_nlink = create_nl_socket(NETLINK_GENERIC);
	if (fd_nlink < 0)
		rd_fatal("error creating Netlink socket: %m");

	current_pid = getpid();

	id = get_family_id(fd_nlink);
	if (!id) {
		rd_err("error getting family id, errno=%d", errno);
		goto err;
	}

	ret = send_cmd(fd_nlink, id, TASKSTATS_CMD_GET,
	               TASKSTATS_CMD_ATTR_REGISTER_CPUMASK, &cpumask, cpumask_len);
	if (ret < 0) {
		rd_err("error sending register cpumask");
		goto err;
	}

	while (1) {
		struct msgtemplate msg;
		struct nlattr *na;

		ret = recv(fd_nlink, &msg, sizeof(msg), 0);
		if (ret < 0) {
			rd_err("nonfatal reply error: errno=%d", errno);
			continue;
		}

		if (msg.n.nlmsg_type == NLMSG_ERROR || !NLMSG_OK((&msg.n), ret)) {
			struct nlmsgerr *err = NLMSG_DATA(&msg);
			rd_fatal("fatal reply error, errno=%d", err->error);
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

	ret = send_cmd(fd_nlink, id, TASKSTATS_CMD_GET,
	               TASKSTATS_CMD_ATTR_DEREGISTER_CPUMASK, &cpumask, cpumask_len);
	if (ret < 0)
		rd_fatal("error sending deregister cpumask");
err:
	close(fd_nlink);

	if (fd_out)
		close(fd_out);

	return 0;
}
