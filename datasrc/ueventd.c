#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/signalfd.h>
#include <sys/epoll.h>
#include <sys/inotify.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <getopt.h>
#include <time.h>
#include <fcntl.h>
#include <error.h>
#include <errno.h>
#include <syslog.h>
#include <dirent.h>

#include "uevent-logging.h"
#include "uevent-pidfile.h"
#include "uevent-epoll.h"
#include "uevent-worker.h"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define DEFAULT_TIMEOUT 1

int fd_ep        = -1;
int fd_signal    = -1;
int fd_eventdir  = -1;
pid_t worker_pid = 0;

struct timespec last, now;

int do_exit = 0;

/* Arguments */
int timeout          = DEFAULT_TIMEOUT;
int log_priority     = 0;
char *logfile        = NULL;
static char *pidfile = NULL;
static int daemonize = 1;

static void
handle_signal(uint32_t signo)
{
	switch (signo) {
		case SIGINT:
		case SIGTERM:
			do_exit = 1;
			break;

		case SIGCHLD:
			while (1) {
				pid_t pid;
				int status;

				if ((pid = waitpid(-1, &status, 0)) < 0)
					break;

				if (pid == worker_pid) {
					dbg("Worker %d exit = %d", pid, WEXITSTATUS(status));
					worker_pid = 0;

					if (clock_gettime(CLOCK_MONOTONIC, &now) < 0)
						fatal("clock_gettime: %m");
					last.tv_sec = now.tv_sec;
				}
			}
			break;

		case SIGHUP:
			break;
	}
}

static void
read_inotify_events(int fd)
{
	ssize_t count;
	size_t i, len = 0;
	void *buf;
	struct inotify_event *ie;

	if (ioctl(fd, FIONREAD, &len) < 0)
		fatal("ioctl: %m");

	ie = buf = malloc(len);
	if (!buf)
		fatal("malloc: allocating %lu bytes: %m", (unsigned long) len);

	count = TEMP_FAILURE_RETRY(read(fd, buf, len));
	if (count < 0)
		fatal("read: unable to read events: %m");

	len = (size_t) count;

	while (len > 0) {
		dbg("Event %s", ie->name);

		// next event
		i = sizeof(struct inotify_event) + ie->len;
		len -= i;
		ie = (void *) ((char *) ie + i);
	}

	free(buf);
}

static int
is_dir_not_empty(char *eventdir)
{
	struct dirent *ent;
	DIR *dir = NULL;
	int ret  = 0;

	if ((dir = opendir(eventdir)) == NULL)
		fatal("opendir: %m");

	errno = 0;
	while ((ent = readdir(dir)) != NULL) {
		if (ent->d_type == DT_REG) {
			ret = 1;
			break;
		}
	}

	if (errno != 0)
		fatal("readdir: %m");

	closedir(dir);

	return ret;
}

int
main(int argc, char **argv)
{
	struct timespec mtime = { 0 };
	sigset_t mask, sigmask_orig;
	int c, fd;
	int ep_timeout   = 0;
	int ignore_timer = 0;
	int new_events   = 0;

	char *eventdir, *prog, **prog_args;

	struct option long_options[] = {
		{ "help", no_argument, 0, 'h' },
		{ "version", no_argument, 0, 'V' },
		{ "foreground", no_argument, 0, 'f' },
		{ "loglevel", required_argument, 0, 'L' },
		{ "logfile", required_argument, 0, 'l' },
		{ "pidfile", required_argument, 0, 'p' },
		{ "timeout", required_argument, 0, 't' },
		{ 0, 0, 0, 0 }
	};

	while ((c = getopt_long(argc, argv, "hVfL:l:p:", long_options, NULL)) != -1) {
		switch (c) {
			case 't':
				timeout = atoi(optarg);
				if (!timeout)
					timeout = DEFAULT_TIMEOUT;
				break;
			case 'p':
				pidfile = optarg;
				break;
			case 'l':
				logfile = optarg;
				break;
			case 'L':
				log_priority = logging_level(optarg);
				break;
			case 'f':
				daemonize = 0;
				break;
			case 'V':
				printf("%s %s\n", PROGRAM_NAME, VERSION);
				return EXIT_SUCCESS;
			default:
			case 'h':
				printf("Usage: %s [options] DIRECTORY PROGRAM [ARGS...]\n"
				       "\nThe utility monitors the DIRECTORY and when\n"
				       "new files appear run the PROGRAM.\n\n"
				       "Options:\n"
				       " -p, --pidfile=FILE   pid file location;\n"
				       " -l, --logfile=FILE   log file;\n"
				       " -L, --loglevel=LVL   logging level;\n"
				       " -t, --timeout=SEC    number of seconds that need to wait"
				       "                      for files before the PROGRAM launch;\n"
				       " -f, --foreground     stay in the foreground;\n"
				       " -V, --version        print program version and exit;\n"
				       " -h, --help           show this text and exit.\n"
				       "\n",
				       PROGRAM_NAME);
				return EXIT_SUCCESS;
		}
	}

	if (optind >= argc)
		error(EXIT_FAILURE, 0, "You must specify the directory");

	eventdir = argv[optind++];

	if (optind >= argc)
		error(EXIT_FAILURE, 0, "You must specify the program");

	prog = canonicalize_file_name(argv[optind]);
	if (!prog)
		error(EXIT_FAILURE, errno, "Bad program");

	argv[optind] = strrchr(prog, '/');
	if (!argv[optind])
		argv[optind] = prog;

	prog_args = argv + optind;

	if (!log_priority)
		log_priority = logging_level("info");

	if (pidfile && check_pid(pidfile))
		error(EXIT_FAILURE, 0, "%s: already running", PROGRAM_NAME);

	if (chdir("/") < 0)
		error(EXIT_FAILURE, errno, "%s: chdir(/)", PROGRAM_NAME);

	close(STDIN_FILENO);

	if ((fd = open("/dev/null", O_RDONLY)) < 0)
		error(EXIT_FAILURE, errno, "%s: open(/dev/null)", PROGRAM_NAME);

	if (fd != STDIN_FILENO) {
		dup2(fd, STDIN_FILENO);
		close(fd);
	}

	if (daemonize && daemon(0, 1) < 0)
		error(EXIT_FAILURE, errno, "%s: daemon", PROGRAM_NAME);

	logging_init();

	info("starting version %s", VERSION);

	if (pidfile && write_pid(pidfile) == 0)
		return EXIT_FAILURE;

	sigfillset(&mask);
	sigprocmask(SIG_SETMASK, &mask, &sigmask_orig);

	sigdelset(&mask, SIGABRT);
	sigdelset(&mask, SIGSEGV);

	if ((fd_ep = epoll_create1(EPOLL_CLOEXEC)) < 0)
		fatal("epoll_create1: %m");

	if ((fd_signal = signalfd(-1, &mask, SFD_NONBLOCK | SFD_CLOEXEC)) < 0)
		fatal("signalfd: %m");

	epollin_add(fd_ep, fd_signal);

	if ((fd_eventdir = inotify_init1(IN_NONBLOCK | IN_CLOEXEC)) < 0)
		fatal("inotify_init1: %m");

	if (inotify_add_watch(fd_eventdir, eventdir, IN_ONLYDIR | IN_DONT_FOLLOW | IN_MOVED_TO | IN_CLOSE_WRITE) < 0)
		fatal("inotify_add_watch: %m");

	epollin_add(fd_ep, fd_eventdir);

	ignore_timer = is_dir_not_empty(eventdir);

	if (clock_gettime(CLOCK_MONOTONIC, &now) < 0)
		fatal("clock_gettime: %m");
	last.tv_sec = now.tv_sec;

	while (!do_exit) {
		struct epoll_event ev[42];
		int i, fdcount;
		ssize_t size;

		if ((fdcount = epoll_wait(fd_ep, ev, ARRAY_SIZE(ev), ep_timeout)) < 0)
			continue;

		if (!ep_timeout)
			ep_timeout = timeout * 1000;

		for (i = 0; i < fdcount; i++) {

			if (!(ev[i].events & EPOLLIN)) {
				continue;

			} else if (ev[i].data.fd == fd_signal) {
				struct signalfd_siginfo fdsi;

				size = TEMP_FAILURE_RETRY(read(fd_signal,
				                               &fdsi, sizeof(struct signalfd_siginfo)));

				if (size != sizeof(struct signalfd_siginfo)) {
					err("unable to read signal info");
					continue;
				}

				handle_signal(fdsi.ssi_signo);

			} else if (ev[i].data.fd == fd_eventdir) {
				read_inotify_events(fd_eventdir);
				new_events += 1;
			}
		}

		if (new_events) {
			struct stat sb;

			new_events = 0;

			if (lstat(eventdir, &sb) < 0)
				fatal("lstat: %s: %m", eventdir);

			if (mtime.tv_sec != sb.st_mtim.tv_sec || mtime.tv_nsec != sb.st_mtim.tv_nsec) {
				if (clock_gettime(CLOCK_MONOTONIC, &now) < 0)
					fatal("clock_gettime: %m");
				last.tv_sec = now.tv_sec;
			}

			mtime.tv_sec  = sb.st_mtim.tv_sec;
			mtime.tv_nsec = sb.st_mtim.tv_nsec;
		}

		if (worker_pid)
			continue;

		if (!ignore_timer) {
			if (clock_gettime(CLOCK_MONOTONIC, &now) < 0)
				fatal("clock_gettime: %m");

			if (now.tv_sec < last.tv_sec || (now.tv_sec - last.tv_sec) < timeout)
				continue;
		}
		ignore_timer = 0;

		if ((worker_pid = spawn_worker(prog, prog_args)) < 0)
			fatal("spawn_worker: %m");

		dbg("Run worker %d", worker_pid);
	}

	epollin_remove(fd_ep, fd_signal);
	epollin_remove(fd_ep, fd_eventdir);

	free(prog);

	if (pidfile)
		remove_pid(pidfile);

	logging_close();

	return EXIT_SUCCESS;
}
