/* SPDX-License-Identifier: GPL-2.0-or-later */

#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/inotify.h>
#include <sys/signalfd.h>
#include <sys/prctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <signal.h>
#include <getopt.h>
#include <errno.h>

#include "ueventd.h"

char *uevent_confdb;
char *filter_dir;
char *uevent_dir;
char *handler_file;
uint64_t session = 0;

static struct watch *watch_list = NULL;
static int pause_all = 0;

enum {
	FD_INOTIFY,
	FD_SIGNAL,
	FD_PIPE,
	FD_MAX,
};

typedef int (*fdhandler_t)(int);

struct fd_handler {
	int fd;
	fdhandler_t fd_handler;
};

struct fd_handler fd_list[FD_MAX];

enum {
	PIPE_READ,
	PIPE_WRITE,
	PIPE_MAX,
};

int pipefd[PIPE_MAX];

#define EV_PAUSE_MASK (IN_CREATE | IN_DELETE | IN_ONLYDIR)
#define EV_ROOT_MASK  (IN_CREATE | IN_ONLYDIR)
#define EV_QUEUE_MASK (IN_CLOSE_WRITE | IN_MOVED_TO | IN_DELETE_SELF)
#define EV_MAX        (FD_MAX * 32)

static int add_queue_dir(int inotifyfd, const char *path, uint32_t mask) __attribute__((nonnull(2)));
static void watch_pauses(int inotifyfd);
static void watch_queues(int inotifyfd);
static int watch_path(int inotifyfd, const char *dir, const char *name, uint32_t mask, int flags) __attribute__((nonnull(2, 3)));
static int unwatch_path(int inotifyfd, ino_t ino);
static void apply_pause(void);
static int process_signal_events(int signfd);
static int process_inotify_events(int inotifyfd);
static int process_pipefd_events(int readfd);
static void setup_signal_fd(struct fd_handler *el)          __attribute__((nonnull(1)));
static void setup_inotify_fd(struct fd_handler *el)         __attribute__((nonnull(1)));
static void setup_pipe_fd(struct fd_handler *el)            __attribute__((nonnull(1)));
static int setup_epoll_fd(struct fd_handler list[FD_MAX]);
static pid_t spawn_worker(int epollfd, struct watch *queue) __attribute__((nonnull(2)));
static inline char *get_param_dir(const char *name)         __attribute__((nonnull(1)));

int add_queue_dir(int inotifyfd, const char *path, uint32_t mask)
{
	int ret;

	errno = 0;
	ret = inotify_add_watch(inotifyfd, path, mask | IN_MASK_CREATE);
	if (ret < 0) {
		if (errno == EEXIST) {
			return -128;
		}
		err("inotify failed to watch: %s: %m", path);
	}
	return ret;
}

int watch_path(int inotifyfd, const char *dir, const char *name, uint32_t mask, int flags)
{
	struct stat st;
	struct watch *new = NULL;
	char *path = NULL;

	xasprintf(&path, "%s/%s", dir, name);

	int wfd = add_queue_dir(inotifyfd, path, mask);
	if (wfd < 0)
		return (wfd == -128 ? 0 : wfd);

	if (stat(path, &st) < 0) {
		err("stat: %s: %m", path);
		goto fail;
	}

	new = xcalloc(1, sizeof(*new) + sizeof(new->q_name[0]) * (strlen(name) + 1));

	strcpy(new->q_name, name);

	new->q_watchfd = wfd;
	new->q_ino     = st.st_ino;
	new->q_flags   = flags;
	new->next      = watch_list;

	if (flags & F_QUEUE_DIR) {
		if (!empty_directory(path))
			new->q_flags |= F_DIRTY;
		new->q_parentfd = pipefd[PIPE_WRITE];
	}

	if (pause_all)
		new->q_flags |= F_PAUSED;

	watch_list = new;

	info("watch path: %s", path);
	free(path);
	return 0;
fail:
	inotify_rm_watch(inotifyfd, wfd);
	free(new);
	free(path);
	return -1;
}

int unwatch_path(int inotifyfd, ino_t ino)
{
	struct watch *prev, *elem;

	prev = NULL;
	elem = watch_list;

	while (elem) {
		if (elem->q_ino == ino) {
			if (prev)
				prev->next = elem->next;
			if (elem == watch_list)
				watch_list = elem->next;
			inotify_rm_watch(inotifyfd, elem->q_watchfd);
			free(elem);
			break;
		}

		prev = elem;
		elem = elem->next;
	}

	return 0;
}

void watch_pauses(int inotifyfd)
{
	char *path = NULL;

	xasprintf(&path, "%s/queue/pause", uevent_confdb);

	if (watch_path(inotifyfd, path, ".", EV_PAUSE_MASK, F_PAUSE_DIR) < 0)
		exit(EXIT_FAILURE);

	free(path);
}

void apply_pause(void)
{
	struct watch *p;
	DIR *dir;
	char *path = NULL;

	xasprintf(&path, "%s/queue/pause", uevent_confdb);

	dir = xopendir(path);
	pause_all = 0;

	for (p = watch_list; p; p = p->next) {
		if (p->q_flags & F_PAUSED)
			p->q_flags &= ~F_PAUSED;
	}

	struct dirent *ent;

	while ((ent = xreaddir(dir, path)) != NULL) {
		if (ent->d_type != DT_DIR || is_dot_dir(ent))
			continue;

		if (!strcmp(ent->d_name, ".all"))
			pause_all = 1;

		for (p = watch_list; p; p = p->next) {
			if ((p->q_flags & F_QUEUE_DIR) &&
			    (pause_all || !strcmp(ent->d_name, p->q_name)))
				p->q_flags |= F_PAUSED;
		}

		if (pause_all)
			break;
	}
	closedir(dir);
	free(path);
}

void watch_queues(int inotifyfd)
{
	DIR *dir = opendir(filter_dir);
	if (!dir)
		fatal("opendir: %s: %m", filter_dir);

	struct dirent *ent;

	while ((ent = xreaddir(dir, filter_dir)) != NULL) {
		if (ent->d_type != DT_DIR || is_dot_dir(ent))
			continue;

		if (watch_path(inotifyfd, filter_dir, ent->d_name, EV_QUEUE_MASK, F_QUEUE_DIR) < 0)
			fatal("unable to start watching: %s/%s", filter_dir, ent->d_name);
	}
	closedir(dir);
}

int process_signal_events(int signfd)
{
	struct watch *p;
	struct signalfd_siginfo fdsi = { 0 };
	ssize_t size;

	size = read_retry(signfd, &fdsi, sizeof(fdsi));
	if (size != sizeof(fdsi)) {
		err("unable to read signal info");
		return 0;
	}

	if (fdsi.ssi_signo == SIGCHLD) {
		while (1) {
			pid_t pid = waitpid_retry(-1, NULL, WNOHANG);

			if (pid <= 0) {
				if (pid < 0 && errno != ECHILD) {
					err("waitpid: %m");
					return -1;
				}
				break;
			}

			for (p = watch_list; p; p = p->next) {
				if (p->q_pid == pid)
					p->q_pid = 0;
			}
		}
		return 0;
	}

	info("got signal %d, exit", fdsi.ssi_signo);
	return -1;
}

void setup_signal_fd(struct fd_handler *el)
{
	sigset_t mask;

	sigemptyset(&mask);

	sigaddset(&mask, SIGHUP);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGQUIT);
	sigaddset(&mask, SIGTERM);
	sigaddset(&mask, SIGCHLD);

	if ((el->fd = signalfd(-1, &mask, SFD_NONBLOCK | SFD_CLOEXEC)) < 0)
		fatal("signalfd: %m");

	if (sigprocmask(SIG_BLOCK, &mask, NULL) < 0)
		fatal("sigprocmask: %m");

	el->fd_handler = process_signal_events;
}

int process_inotify_events(int inotifyfd)
{
	char buf[4096] __attribute__ ((aligned(__alignof__(struct inotify_event))));

	while (1) {
		ssize_t len = read_retry(inotifyfd, buf, sizeof(buf));

		if (len < 0) {
			if (errno == EAGAIN)
				break;
			fatal("read: %m");
		} else if (!len) {
			break;
		}

		struct inotify_event *event;
		char *ptr;

		for (ptr = buf; ptr < buf + len; ptr += sizeof(*event) + event->len) {
			struct watch *p;

			event = (struct inotify_event *) ptr;

			for (p = watch_list; p && (p->q_watchfd != event->wd); p = p->next);
			if (!p)
				continue;

			if (event->mask & IN_DELETE_SELF) {
				info("unwatch path: %s", p->q_name);
				unwatch_path(inotifyfd, p->q_ino);
				continue;
			}

			if (p->q_flags & F_ROOT_DIR) {
				if (event->mask & IN_CREATE)
					watch_queues(inotifyfd);
				continue;
			}

			if (p->q_flags & F_PAUSE_DIR) {
				if (event->mask & IN_CREATE)
					info("%s: queue paused", event->name);
				else if (event->mask & IN_DELETE)
					info("%s: queue unpaused", event->name);
				apply_pause();
				continue;
			}

			if (!(p->q_flags & F_DIRTY))
				p->q_flags |= F_DIRTY;
		}
	}
	return 0;
}

void setup_inotify_fd(struct fd_handler *el)
{
	if ((el->fd = inotify_init1(IN_NONBLOCK | IN_CLOEXEC)) < 0)
		fatal("inotify_init1: %m");

	el->fd_handler = process_inotify_events;
}

int process_pipefd_events(int readfd)
{
	while (1) {
		struct watch *p;
		ssize_t len;
		int value = 0;

		errno = 0;
		len = read_retry(readfd, &value, sizeof(value));

		if (len < 0) {
			if (errno == EAGAIN)
				break;
			fatal("read(pipe): %m");
		} else if (!len) {
			break;
		}

		for (p = watch_list; p; p = p->next) {
			if (!(p->q_flags & F_QUEUE_DIR) || p->q_watchfd != value)
				continue;
			if (!(p->q_flags & F_DIRTY))
				p->q_flags |= F_DIRTY;
			break;
		}
	}

	return 0;
}

void setup_pipe_fd(struct fd_handler *el)
{
	if (pipe2(pipefd, O_NONBLOCK | O_CLOEXEC) < 0)
		fatal("pipe2: %m");

	el->fd = pipefd[PIPE_READ];
	el->fd_handler = process_pipefd_events;
}

int setup_epoll_fd(struct fd_handler list[FD_MAX])
{
	int epollfd;
	struct epoll_event ev = { .events = EPOLLIN };

	if ((epollfd = epoll_create1(EPOLL_CLOEXEC)) < 0)
		fatal("epoll_create1: %m");

	for (int i = 0; i < FD_MAX; i++) {
		ev.data.fd = list[i].fd;

		if (epoll_ctl(epollfd, EPOLL_CTL_ADD, list[i].fd, &ev) < 0)
			fatal("epoll_ctl: %m");
	}

	return epollfd;
}

pid_t spawn_worker(int epollfd, struct watch *queue)
{
	pid_t pid;

	session++;

	pid = fork();
	if (pid < 0) {
		err("fork: %m");
		return 0;
	} else if (pid > 0) {
		return pid;
	}

	if (prctl(PR_SET_PDEATHSIG, SIGKILL) < 0)
		fatal("prctl(PR_SET_PDEATHSIG): %m");

	for (int i = 0; i < FD_MAX; i++) {
		if (fd_list[i].fd >= 0)
			close(fd_list[i].fd);
	}
	close(epollfd);

	sigset_t mask;
	sigemptyset(&mask);

	if (sigprocmask(SIG_SETMASK, &mask, NULL) < 0)
		fatal("sigprocmask: %m");

	process_events(queue);

	return 0;
}

char *get_param_dir(const char *name)
{
	char *value = getenv(name);
	if (!value)
		fatal("please set `%s' env variable", name);
	return value;
}

int main(int argc, char **argv)
{
	struct watch *e, *n;
	int i, epollfd;
	int loglevel = LOG_INFO;

	while ((i = getopt(argc, argv, "hl:")) != -1) {
		switch (i) {
			case 'h':
				fprintf(stderr, "Usage: %s [-l loglevel] handler-prog\n",
				        program_invocation_short_name);
				exit(0);
				break;
			case 'l':
				loglevel = logging_level(optarg);
				break;
			default:
				exit(1);
				break;
		}
	}

	if (optind == argc)
		fatal("specify handler program");

	logging_init(loglevel);

	info("starting server ...");

	filter_dir = get_param_dir("filterdir");
	uevent_dir = get_param_dir("ueventdir");
	uevent_confdb = get_param_dir("uevent_confdb");
	handler_file = argv[optind++];

	setup_inotify_fd(&fd_list[FD_INOTIFY]);
	setup_signal_fd(&fd_list[FD_SIGNAL]);
	setup_pipe_fd(&fd_list[FD_PIPE]);

	epollfd = setup_epoll_fd(fd_list);

	watch_pauses(fd_list[FD_INOTIFY].fd);

	if (watch_path(fd_list[FD_INOTIFY].fd, filter_dir, ".", EV_ROOT_MASK, F_ROOT_DIR) < 0)
		exit(EXIT_FAILURE);

	watch_queues(fd_list[FD_INOTIFY].fd);
	apply_pause();

	while (1) {
		struct epoll_event ev[EV_MAX];
		int fdcount;

		errno = 0;
		fdcount = epoll_wait(epollfd, ev, EV_MAX, 250);

		if (fdcount < 0) {
			if (errno == EINTR)
				continue;
			fatal("epoll_wait: %m");
		}

		for (i = 0; i < fdcount; i++) {
			if (!(ev[i].events & EPOLLIN))
				continue;
			for (int k = 0; k < FD_MAX; k++) {
				if (ev[i].data.fd != fd_list[k].fd)
					continue;
				if (fd_list[k].fd_handler(fd_list[k].fd) != 0)
					goto done;
			}
		}

		for (e = watch_list; e; e = e->next) {
			if (!(e->q_flags & F_QUEUE_DIR) || !(e->q_flags & F_DIRTY) || (e->q_pid != 0) || (e->q_flags & F_PAUSED))
				continue;
			e->q_flags &= ~F_DIRTY;
			e->q_pid = spawn_worker(epollfd, e);
		}
	}
done:
	for (e = watch_list, n = NULL; e; e = n) {
		n = e->next;
		if (e->q_pid)
			kill(e->q_pid, SIGKILL);
		inotify_rm_watch(fd_list[FD_INOTIFY].fd, e->q_watchfd);
		free(e);
	}

	for (i = 0; i < FD_MAX; i++) {
		if (fd_list[i].fd >= 0) {
			if (epoll_ctl(epollfd, EPOLL_CTL_DEL, fd_list[i].fd, NULL) < 0)
				err("epoll_ctl: %m");
			close(fd_list[i].fd);
		}
	}
	close(epollfd);
	logging_close();

	return EXIT_SUCCESS;
}
