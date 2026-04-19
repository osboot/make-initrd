// SPDX-License-Identifier: GPL-3.0-or-later

#include "config.h"

#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/inotify.h>
#include <sys/signalfd.h>
#include <sys/timerfd.h>
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
#include <time.h>
#include <limits.h>

#include "rd/memory.h"
#include "rd/logging.h"
#include "ueventd.h"

#define DEFAULT_LOGFILE "/var/log/ueventd.log"

char *uevent_confdb;
char *filter_dir;
char *timer_dir;
char *uevent_dir;
char *handler_file;
uint64_t session = 0;

static struct watch *watch_list = NULL;
static int pause_all = 0;

enum {
	FD_INOTIFY,
	FD_SIGNAL,
	FD_PIPE,
	FD_TIMER,
	FD_MAX,
};

typedef int (*fdhandler_t)(int);

struct fd_handler {
	int fd;
	fdhandler_t fd_handler;
};

struct fd_handler fd_handler_list[FD_MAX];

enum {
	PIPE_READ,
	PIPE_WRITE,
	PIPE_MAX,
};

static int pipefd[PIPE_MAX];
static int dirty_queues = 0;
static int timer_changed = 0;

#define EV_PAUSE_MASK (IN_CREATE | IN_DELETE | IN_ONLYDIR)
#define EV_ROOT_MASK  (IN_CREATE | IN_ONLYDIR)
#define EV_QUEUE_MASK (IN_CLOSE_WRITE | IN_MOVED_TO | IN_DELETE_SELF)
#define EV_TIMER_MASK (IN_CLOSE_WRITE | IN_MOVED_TO | IN_DELETE_SELF)
#define EV_MAX        (FD_MAX * 32)

static int add_queue_dir(int inotifyfd, const char *path, uint32_t mask) __attribute__((nonnull(2)));
static void watch_pauses(int inotifyfd);
static void watch_queues(int inotifyfd);
static void watch_timer_queues(int inotifyfd);
static int watch_path(int inotifyfd, const char *dir, const char *name, uint32_t mask, int flags) __attribute__((nonnull(2, 3)));
static int unwatch_path(int inotifyfd, ino_t ino);
static void apply_pause(void);
static int process_signal_events(int signfd);
static int process_inotify_events(int inotifyfd);
static int process_pipefd_events(int readfd);
static int process_timerfd_events(int timerfd);
static void setup_signal_fd(struct fd_handler *el)          __attribute__((nonnull(1)));
static void setup_inotify_fd(struct fd_handler *el)         __attribute__((nonnull(1)));
static void setup_pipe_fd(struct fd_handler *el)            __attribute__((nonnull(1)));
static void setup_timer_fd(struct fd_handler *el)           __attribute__((nonnull(1)));
static int setup_epoll_fd(struct fd_handler list[FD_MAX]);
static pid_t spawn_worker(int epollfd, struct watch *queue) __attribute__((nonnull(2)));
static inline char *get_param_dir(const char *name)         __attribute__((nonnull(1)));
static void update_timerfd(int inotifyfd, int timerfd);
static struct watch *find_queue_watch(const char *name)     __attribute__((nonnull(1)));
static void mark_queue_dirty(struct watch *queue)           __attribute__((nonnull(1)));

int add_queue_dir(int inotifyfd, const char *path, uint32_t mask)
{
	int ret;

	errno = 0;
	ret = inotify_add_watch(inotifyfd, path, mask | IN_MASK_CREATE);
	if (ret < 0) {
		if (errno == EEXIST) {
			return -128;
		}
		rd_err("inotify failed to watch: %s: %m", path);
	}
	return ret;
}

int watch_path(int inotifyfd, const char *dir, const char *name, uint32_t mask, int flags)
{
	struct stat st;
	struct watch *new = NULL;

	char *path = rd_asprintf_or_die("%s/%s", dir, name);

	int wfd = add_queue_dir(inotifyfd, path, mask);
	if (wfd < 0) {
		free(path);
		return (wfd == -128 ? 0 : wfd);
	}

	if (stat(path, &st) < 0) {
		rd_err("stat: %s: %m", path);
		goto fail;
	}

	new = rd_calloc_or_die(1, sizeof(*new) + sizeof(new->q_name[0]) * (strlen(name) + 1));

	strcpy(new->q_name, name);

	new->q_watchfd = wfd;
	new->q_ino     = st.st_ino;
	new->q_flags   = flags;
	new->next      = watch_list;

	if (flags & F_QUEUE_DIR) {
		if (!empty_directory(path)) {
			new->q_flags |= F_DIRTY;
			dirty_queues++;
		}
		new->q_parentfd = pipefd[PIPE_WRITE];
	}

	if ((flags & F_TIMER_DIR) && !empty_directory(path))
		timer_changed = 1;

	if (pause_all)
		new->q_flags |= F_PAUSED;

	watch_list = new;

	rd_info("watch path: %s", path);
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
	char *path = rd_asprintf_or_die("%s/queue/pause", uevent_confdb);

	if (watch_path(inotifyfd, path, ".", EV_PAUSE_MASK, F_PAUSE_DIR) < 0)
		exit(EXIT_FAILURE);

	free(path);
}

void apply_pause(void)
{
	struct watch *p;
	DIR *dir;
	char *path = rd_asprintf_or_die("%s/queue/pause", uevent_confdb);

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
		rd_fatal("opendir: %s: %m", filter_dir);

	struct dirent *ent;

	while ((ent = xreaddir(dir, filter_dir)) != NULL) {
		if (ent->d_type != DT_DIR || is_dot_dir(ent))
			continue;

		if (watch_path(inotifyfd, filter_dir, ent->d_name, EV_QUEUE_MASK, F_QUEUE_DIR) < 0)
			rd_fatal("unable to start watching: %s/%s", filter_dir, ent->d_name);
	}
	closedir(dir);
}

void watch_timer_queues(int inotifyfd)
{
	DIR *dir = opendir(timer_dir);
	if (!dir)
		rd_fatal("opendir: %s: %m", timer_dir);

	struct dirent *ent;

	while ((ent = xreaddir(dir, timer_dir)) != NULL) {
		if (ent->d_type != DT_DIR || is_dot_dir(ent))
			continue;

		if (watch_path(inotifyfd, timer_dir, ent->d_name, EV_TIMER_MASK, F_TIMER_DIR) < 0)
			rd_fatal("unable to start watching: %s/%s", timer_dir, ent->d_name);
	}
	closedir(dir);
}

struct watch *find_queue_watch(const char *name)
{
	struct watch *p;

	for (p = watch_list; p; p = p->next) {
		if ((p->q_flags & F_QUEUE_DIR) && !strcmp(p->q_name, name))
			return p;
	}

	return NULL;
}

void mark_queue_dirty(struct watch *queue)
{
	if (!(queue->q_flags & F_DIRTY)) {
		queue->q_flags |= F_DIRTY;
		dirty_queues++;
	}
}

static unsigned long long parse_timer_due(const char *name)
{
	char *end = NULL;
	unsigned long long due;

	errno = 0;
	due = strtoull(name, &end, 10);
	if (errno || !end || *end != '.' || end == name)
		return 0;

	return due;
}

static void ensure_queue_dir(int inotifyfd, const char *name)
{
	char *path = rd_asprintf_or_die("%s/%s", filter_dir, name);
	char *tmp = rd_asprintf_or_die("%s/.tmp", path);

	if (mkdir(path, 0755) < 0 && errno != EEXIST)
		rd_fatal("mkdir: %s: %m", path);
	if (mkdir(tmp, 0755) < 0 && errno != EEXIST)
		rd_fatal("mkdir: %s: %m", tmp);

	if (!find_queue_watch(name) &&
	    watch_path(inotifyfd, filter_dir, name, EV_QUEUE_MASK, F_QUEUE_DIR) < 0)
		rd_fatal("unable to start watching: %s", path);

	free(tmp);
	free(path);
}

static void move_timer_event(int inotifyfd, const char *queue_name, const char *qpath, const char *timer_name)
{
	const char *event_name = strchr(timer_name, '.');
	struct watch *queue;
	char *src, *dst;

	if (!event_name || !event_name[1])
		return;
	event_name++;

	ensure_queue_dir(inotifyfd, queue_name);

	src = rd_asprintf_or_die("%s/%s", qpath, timer_name);
	dst = rd_asprintf_or_die("%s/%s/%s", filter_dir, queue_name, event_name);

	if (rename(src, dst) < 0) {
		if (errno != ENOENT)
			rd_err("rename `%s' -> `%s': %m", src, dst);
	} else {
		queue = find_queue_watch(queue_name);
		if (queue)
			mark_queue_dirty(queue);
	}

	free(dst);
	free(src);
}

void update_timerfd(int inotifyfd, int timerfd)
{
	struct timespec now = { 0 };
	DIR *dir;
	struct dirent *queue_ent;
	unsigned long long next = ULLONG_MAX;
	struct itimerspec its = { 0 };

	if (clock_gettime(CLOCK_BOOTTIME, &now) < 0)
		rd_fatal("clock_gettime(CLOCK_BOOTTIME): %m");

	dir = opendir(timer_dir);
	if (!dir)
		rd_fatal("opendir: %s: %m", timer_dir);

	while ((queue_ent = xreaddir(dir, timer_dir)) != NULL) {
		DIR *qdir;
		struct dirent *timer_ent;
		char *qpath;

		if (queue_ent->d_type != DT_DIR || is_dot_dir(queue_ent))
			continue;

		qpath = rd_asprintf_or_die("%s/%s", timer_dir, queue_ent->d_name);
		qdir = xopendir(qpath);

		while ((timer_ent = xreaddir(qdir, qpath)) != NULL) {
			unsigned long long due;

			if (timer_ent->d_name[0] == '.' || timer_ent->d_type != DT_REG)
				continue;

			due = parse_timer_due(timer_ent->d_name);
			if (!due)
				continue;

			if (due > (unsigned long long) now.tv_sec) {
				if (due < next)
					next = due;
				continue;
			}

			move_timer_event(inotifyfd, queue_ent->d_name, qpath, timer_ent->d_name);
		}
		closedir(qdir);
		free(qpath);
	}
	closedir(dir);

	if (next != ULLONG_MAX)
		its.it_value.tv_sec = (time_t) next;

	if (timerfd_settime(timerfd, TFD_TIMER_ABSTIME, &its, NULL) < 0)
		rd_fatal("timerfd_settime: %m");

	timer_changed = 0;
}

int process_signal_events(int signfd)
{
	struct watch *p;
	struct signalfd_siginfo fdsi = { 0 };
	ssize_t size;

	size = read_retry(signfd, &fdsi, sizeof(fdsi));
	if (size != sizeof(fdsi)) {
		rd_err("unable to read signal info");
		return 0;
	}

	if (fdsi.ssi_signo == SIGCHLD) {
		while (1) {
			pid_t pid = waitpid_retry(-1, NULL, WNOHANG);

			if (pid <= 0) {
				if (pid < 0 && errno != ECHILD) {
					rd_err("waitpid: %m");
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

	rd_info("got signal %d, exit", fdsi.ssi_signo);
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
		rd_fatal("signalfd: %m");

	if (sigprocmask(SIG_BLOCK, &mask, NULL) < 0)
		rd_fatal("sigprocmask: %m");

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
			rd_fatal("read: %m");
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
				rd_info("unwatch path: %s", p->q_name);
				unwatch_path(inotifyfd, p->q_ino);
				continue;
			}

			if (p->q_flags & F_ROOT_DIR) {
				if (event->mask & IN_CREATE)
					watch_queues(inotifyfd);
				continue;
			}

			if (p->q_flags & F_TIMER_ROOT) {
				if (event->mask & IN_CREATE)
					watch_timer_queues(inotifyfd);
				timer_changed = 1;
				continue;
			}

			if (p->q_flags & F_PAUSE_DIR) {
				if (event->mask & IN_CREATE)
					rd_info("%s: queue paused", event->name);
				else if (event->mask & IN_DELETE)
					rd_info("%s: queue unpaused", event->name);
				apply_pause();
				continue;
			}

			if (p->q_flags & F_TIMER_DIR) {
				timer_changed = 1;
				continue;
			}

			mark_queue_dirty(p);
		}
	}
	return 0;
}

void setup_inotify_fd(struct fd_handler *el)
{
	if ((el->fd = inotify_init1(IN_NONBLOCK | IN_CLOEXEC)) < 0)
		rd_fatal("inotify_init1: %m");

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
			rd_fatal("read(pipe): %m");
		} else if (!len) {
			break;
		}

		for (p = watch_list; p; p = p->next) {
			if (!(p->q_flags & F_QUEUE_DIR) || p->q_watchfd != value)
				continue;
			if (!(p->q_flags & F_DIRTY)) {
				p->q_flags |= F_DIRTY;
				dirty_queues++;
			}
			break;
		}
	}

	return 0;
}

void setup_pipe_fd(struct fd_handler *el)
{
	if (pipe2(pipefd, O_NONBLOCK | O_CLOEXEC) < 0)
		rd_fatal("pipe2: %m");

	el->fd = pipefd[PIPE_READ];
	el->fd_handler = process_pipefd_events;
}

int process_timerfd_events(int timerfd)
{
	uint64_t expirations;
	ssize_t len;

	len = read_retry(timerfd, &expirations, sizeof(expirations));
	if (len < 0) {
		if (errno == EAGAIN)
			return 0;
		rd_fatal("read(timerfd): %m");
	}

	timer_changed = 1;
	return 0;
}

void setup_timer_fd(struct fd_handler *el)
{
	if ((el->fd = timerfd_create(CLOCK_BOOTTIME, TFD_NONBLOCK | TFD_CLOEXEC)) < 0)
		rd_fatal("timerfd_create: %m");

	el->fd_handler = process_timerfd_events;
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

pid_t spawn_worker(int epollfd, struct watch *queue)
{
	pid_t pid;

	session++;

	pid = fork();
	if (pid < 0) {
		rd_err("fork: %m");
		return 0;
	} else if (pid > 0) {
		return pid;
	}

	if (prctl(PR_SET_PDEATHSIG, SIGKILL) < 0)
		rd_fatal("prctl(PR_SET_PDEATHSIG): %m");

	for (int i = 0; i < FD_MAX; i++) {
		if (fd_handler_list[i].fd >= 0)
			close(fd_handler_list[i].fd);
	}
	close(epollfd);

	sigset_t mask;
	sigemptyset(&mask);

	if (sigprocmask(SIG_SETMASK, &mask, NULL) < 0)
		rd_fatal("sigprocmask: %m");

	process_events(queue);

	return 0;
}

char *get_param_dir(const char *name)
{
	char *value = getenv(name);
	if (!value)
		rd_fatal("please set `%s' env variable", name);
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
				loglevel = rd_logging_level(optarg);
				break;
			default:
				exit(1);
				break;
		}
	}

	if (optind == argc)
		rd_fatal("specify handler program");

	rd_log_setup_stderr(DEFAULT_LOGFILE);
	rd_logging_init(fileno(stderr), loglevel, program_invocation_short_name);

	rd_info("starting server ...");

	filter_dir = get_param_dir("filterdir");
	uevent_dir = get_param_dir("ueventdir");
	uevent_confdb = get_param_dir("uevent_confdb");
	handler_file = argv[optind++];
	timer_dir = get_param_dir("timerdir");

	setup_inotify_fd(&fd_handler_list[FD_INOTIFY]);
	setup_signal_fd(&fd_handler_list[FD_SIGNAL]);
	setup_pipe_fd(&fd_handler_list[FD_PIPE]);
	setup_timer_fd(&fd_handler_list[FD_TIMER]);

	epollfd = setup_epoll_fd(fd_handler_list);

	watch_pauses(fd_handler_list[FD_INOTIFY].fd);

	if (watch_path(fd_handler_list[FD_INOTIFY].fd, filter_dir, ".", EV_ROOT_MASK, F_ROOT_DIR) < 0)
		exit(EXIT_FAILURE);
	if (watch_path(fd_handler_list[FD_INOTIFY].fd, timer_dir, ".", EV_ROOT_MASK, F_TIMER_ROOT) < 0)
		exit(EXIT_FAILURE);

	watch_queues(fd_handler_list[FD_INOTIFY].fd);
	watch_timer_queues(fd_handler_list[FD_INOTIFY].fd);
	apply_pause();
	update_timerfd(fd_handler_list[FD_INOTIFY].fd, fd_handler_list[FD_TIMER].fd);

	while (1) {
		struct epoll_event ev[EV_MAX];
		int fdcount;

		errno = 0;
		fdcount = epoll_wait(epollfd, ev, EV_MAX, (dirty_queues ? 250 : -1));

		if (fdcount < 0) {
			if (errno == EINTR)
				continue;
			rd_fatal("epoll_wait: %m");
		}

		for (i = 0; i < fdcount; i++) {
			if (!(ev[i].events & EPOLLIN))
				continue;

			struct fd_handler *fde = ev[i].data.ptr;

			if (fde->fd_handler(fde->fd) != 0)
				goto done;
		}

		if (timer_changed)
			update_timerfd(fd_handler_list[FD_INOTIFY].fd, fd_handler_list[FD_TIMER].fd);

		for (e = watch_list; e; e = e->next) {
			if (!(e->q_flags & F_QUEUE_DIR) || !(e->q_flags & F_DIRTY) || (e->q_pid != 0) || (e->q_flags & F_PAUSED))
				continue;

			if (dirty_queues == 0)
				rd_fatal("BUG: dirty_queues == 0, but dirty queues are present");

			dirty_queues--;

			e->q_flags &= ~F_DIRTY;
			e->q_pid = spawn_worker(epollfd, e);
		}
	}
done:
	for (e = watch_list, n = NULL; e; e = n) {
		n = e->next;
		if (e->q_pid)
			kill(e->q_pid, SIGKILL);
		inotify_rm_watch(fd_handler_list[FD_INOTIFY].fd, e->q_watchfd);
		free(e);
	}

	for (i = 0; i < FD_MAX; i++) {
		if (fd_handler_list[i].fd >= 0) {
			if (epoll_ctl(epollfd, EPOLL_CTL_DEL, fd_handler_list[i].fd, NULL) < 0)
				rd_err("epoll_ctl: %m");
			close(fd_handler_list[i].fd);
		}
	}
	close(epollfd);
	rd_logging_close();

	return EXIT_SUCCESS;
}
