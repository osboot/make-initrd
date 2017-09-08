#include <sys/wait.h>
#include <sys/prctl.h>
#include <sys/mount.h>
#include <sys/signalfd.h>

#include <sched.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include <errno.h>
#include <error.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <grp.h> // setgroups

#include "container.h"

int verbose = 0;
const char *program_subname;

struct container {
	char **argv;
	char *root;
	cap_t caps;
	int nice;
	int no_new_privs;
	int unshare_flags;
	uint64_t remap_uid;
	uint64_t remap_gid;
	struct mapfile *envs;
	struct mapfile *devs;
	struct mntent **mounts;
};

const char short_opts[]         = "vVhU:u:g:c:m:e:P:";
const struct option long_opts[] = {
	{ "cap-add", required_argument, NULL, 'A' },
	{ "cap-drop", required_argument, NULL, 'D' },
	{ "mount", required_argument, NULL, 'm' },
	{ "setenv", required_argument, NULL, 'e' },
	{ "uid", required_argument, NULL, 'u' },
	{ "gid", required_argument, NULL, 'g' },
	{ "writepid", required_argument, NULL, 'P' },
	{ "unshare", required_argument, NULL, 'U' },
	{ "help", no_argument, NULL, 'h' },
	{ "verbose", no_argument, NULL, 'v' },
	{ "version", no_argument, NULL, 'V' },
	{ "no-new-privs", no_argument, NULL, 1 },
	{ "nice", required_argument, NULL, 2 },
	{ "devices", required_argument, NULL, 3 },
	{ "environ", required_argument, NULL, 4 },
	{ NULL, 0, NULL, 0 }
};

static void __attribute__((noreturn))
usage(int code)
{
	dprintf(STDOUT_FILENO,
	        "Usage: %s [options] [--] newroot command [arguments...]\n"
	        "\n"
	        "Options:\n"
	        " --no-new-privs        run container with PR_SET_NO_NEW_PRIVS flag\n"
	        " --nice=NUM            change process priority\n"
	        " --environ=FILE        set environment variables listed in the FILE\n"
	        " --devices=FILE        create devices listed in the FILE\n"
	        " -U, --unshare=STR     unshare everything I know\n"
	        " -P, --writepid=FILE   write pid to file\n"
	        " -A, --cap-add=list    add Linux capabilities\n"
	        " -D, --cap-drop=list   drop Linux capabilities\n"
	        " -u, --uid=UID         exposes the mapping of user IDs\n"
	        " -g, --gid=GID         exposes the mapping of group IDs\n"
	        " -e, --setenv=FILE     sets new environ from file\n"
	        " -m, --mount=FSTAB     mounts filesystems using FSTAB file\n"
	        " -h, --help            display this help and exit\n"
	        " -v, --verbose         print a message for each action\n"
	        " -V, --version         output version information and exit\n"
	        "\n",
	        program_invocation_short_name);
	exit(code);
}

static inline void __attribute__((noreturn))
print_version_and_exit(void)
{
	dprintf(STDOUT_FILENO, "%s v1\n", program_invocation_short_name);
	exit(EXIT_SUCCESS);
}

static void
my_error_print_progname(void)
{
	fprintf(stderr, "%s: ", program_invocation_short_name);
}

static void
print_program_subname(void)
{
	fprintf(stderr, "%s: %s: ", program_invocation_short_name, program_subname);
}

static void
append_pid(const char *filename, pid_t pid)
{
	int fd;

	if ((fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0644)) == -1)
		error(EXIT_FAILURE, errno, "open: %s", filename);

	if (!dprintf(fd, "%d\n", pid))
		error(EXIT_FAILURE, errno, "dprintf: %s", filename);

	fsync(fd);
	close(fd);
}

static void
write_pipe(const int fd)
{
	if (write(fd, "x", 1) < 0)
		error(EXIT_FAILURE, errno, "write");
	fsync(fd);
}

static void
read_pipe(const int fd)
{
	char c;
	if (read(fd, &c, 1) < 0)
		error(EXIT_FAILURE, errno, "read");
}

static void
free_data(struct container *data)
{
	size_t i;

	for (i = 0; data->mounts && data->mounts[i]; i++)
		free_mntent(data->mounts[i]);

	if (data->mounts)
		xfree(data->mounts);

	if (data->caps)
		cap_free(data->caps);
}

static int
container_parent(pid_t child_pid, int rpipe, int wpipe, struct container *data)
{
	int i, rc;
	pid_t init_pid, prehook_pid, hook_pid;
	sigset_t mask;
	int fd_ep, fd_signal;
	int ep_timeout = 0;

	program_subname      = "parent";
	error_print_progname = print_program_subname;

	prehook_pid = hook_pid = 0;

	free_data(data);

	if (read(rpipe, &init_pid, sizeof(init_pid)) != sizeof(init_pid))
		error(EXIT_FAILURE, errno, "read init pid");

	setenvf("CONTAINER_PID", "%d", init_pid);

	// wait for client
	read_pipe(rpipe);

	if (data->remap_uid || data->remap_gid) {
		uid_t real_euid = geteuid();
		gid_t real_egid = getegid();

		/* since Linux 3.19 unprivileged writing of /proc/self/gid_map
		 * has s been disabled unless /proc/self/setgroups is written
		 * first to permanently disable the ability to call setgroups
		 * in that user namespace.
		 */
		setgroups_control(init_pid, "deny");

		map_id("uid_map", init_pid, data->remap_uid, real_euid);
		map_id("gid_map", init_pid, data->remap_gid, real_egid);
	}

	sigfillset(&mask);
	sigprocmask(SIG_SETMASK, &mask, NULL);

	sigdelset(&mask, SIGABRT);
	sigdelset(&mask, SIGSEGV);

	if ((fd_signal = signalfd(-1, &mask, SFD_NONBLOCK | SFD_CLOEXEC)) < 0)
		error(EXIT_FAILURE, errno, "signalfd");

	fd_ep = epollin_init();
	epollin_add(fd_ep, fd_signal);

	rc = EXIT_SUCCESS;
	while (1) {
		struct epoll_event ev[42];
		int fdcount;
		ssize_t size;

		errno = 0;
		if ((fdcount = epoll_wait(fd_ep, ev, ARRAY_SIZE(ev), ep_timeout)) < 0) {
			if (errno == EINTR)
				continue;

			error(EXIT_FAILURE, errno, "epoll_wait");
		}

		if (!fdcount) {
			if (!prehook_pid) {
				prehook_pid = run_hook("PRERUN");
				continue;

			} else if (prehook_pid == -2) {
				prehook_pid = -3;

				// allow client to start
				write_pipe(wpipe);

				continue;
			}

			if (!child_pid && hook_pid <= 0)
				goto done;

			ep_timeout = 1000;

		} else
			for (i = 0; i < fdcount; i++) {
				if (!(ev[i].events & EPOLLIN)) {
					continue;

				} else if (ev[i].data.fd == fd_signal) {
					pid_t pid;
					struct signalfd_siginfo fdsi;
					int status = 0;

					size = TEMP_FAILURE_RETRY(read(fd_signal, &fdsi, sizeof(struct signalfd_siginfo)));

					if (size != sizeof(struct signalfd_siginfo)) {
						error(EXIT_SUCCESS, 0, "unable to read signal info");
						continue;
					}

					switch (fdsi.ssi_signo) {
						case SIGCHLD:
							if ((pid = waitpid(-1, &status, 0)) < 0)
								error(EXIT_FAILURE, errno, "waitpid");

							if (pid == child_pid) {
								child_pid = 0;
								hook_pid  = run_hook("POSTRUN");
								rc        = WEXITSTATUS(status);
							} else if (pid == prehook_pid) {
								prehook_pid = -2;
							} else if (pid == hook_pid) {
								hook_pid = 0;
							}
							break;

						case SIGUSR1:
							hook_pid = run_hook("USR1");
							break;

						case SIGHUP:
							break;

						case SIGTERM:
							goto done;
					}
				}
			}
	}
done:
	if (fd_ep >= 0) {
		epollin_remove(fd_ep, fd_signal);
		close(fd_ep);
	}

	close(rpipe);
	close(wpipe);

	return rc;
}

static int
conatainer_child(struct container *data, int rpipe, int wpipe)
{
	program_subname      = "child";
	error_print_progname = print_program_subname;

	if (prctl(PR_SET_PDEATHSIG, SIGKILL) < 0)
		error(EXIT_FAILURE, errno, "prctl(PR_SET_PDEATHSIG)");

	if (data->unshare_flags & CLONE_NEWNS) {
		if (mount("/", "/", "none", MS_PRIVATE | MS_REC, NULL) < 0 && errno != EINVAL)
			error(EXIT_FAILURE, errno, "mount(MS_PRIVATE): %s", data->root);

		if (data->mounts) {
			do_mount(data->root, data->mounts);
			free(data->mounts);
		}
	}

	if (data->unshare_flags & CLONE_NEWUSER) {
		if (unshare(CLONE_NEWUSER) < 0)
			error(EXIT_FAILURE, errno, "unshare(CLONE_NEWUSER)");

		if (verbose)
			unshare_print_flags(CLONE_NEWUSER);
	}

	if (data->unshare_flags & CLONE_NEWNET)
		setup_network();

	// client notify parent that we are ready to run
	write_pipe(wpipe);

	// client waiting for permission to start
	read_pipe(rpipe);

	if (chroot(data->root) < 0)
		error(EXIT_FAILURE, errno, "chroot");

	if (chdir("/") < 0)
		error(EXIT_FAILURE, errno, "chdir");

	if (setsid() < 0)
		error(EXIT_FAILURE, errno, "setsid");

	if (nice(data->nice) < 0)
		error(EXIT_FAILURE, errno, "nice: %d", data->nice);

	if (data->devs)
		make_devices(data->devs);

	if (data->caps)
		apply_caps(data->caps);

	if (prctl(PR_SET_KEEPCAPS, 1) < 0)
		error(EXIT_FAILURE, errno, "prctl(PR_SET_KEEPCAPS)");

	if (data->no_new_privs && prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) < 0)
		error(EXIT_FAILURE, errno, "prctl(PR_SET_NO_NEW_PRIVS)");

	if (verbose)
		dprintf(STDOUT_FILENO, "welcome to chroot: %s\n", data->argv[0]);

	clearenv();

	if (data->envs)
		load_environ(data->envs);

	cloexec_fds();

	execvp(data->argv[0], data->argv);
	error(EXIT_FAILURE, errno, "execvp");

	return EXIT_FAILURE;
}

int
main(int argc, char **argv)
{
	int c;
	pid_t pid;
	char *apid            = NULL;
	struct container data = {};
	struct mapfile envs   = {};
	struct mapfile devs   = {};

	int pipe_1[2]; // client reads from it, parent writes
	int pipe_2[2]; // parent reads from it, client writes

	data.unshare_flags = CLONE_FS;

	error_print_progname = my_error_print_progname;

	while ((c = getopt_long(argc, argv, short_opts, long_opts, NULL)) != EOF) {
		switch (c) {
			case 'h':
				usage(EXIT_SUCCESS);
				break;
			case 1:
				data.no_new_privs = 1;
				break;
			case 2:
				errno     = 0;
				data.nice = (int) strtol(optarg, NULL, 10);
				if (errno == ERANGE)
					error(EXIT_FAILURE, 0, "bad value: %s", optarg);
				break;
			case 3:
				open_map(optarg, &devs);
				data.devs = &devs;
				break;
			case 4:
				open_map(optarg, &envs);
				data.envs = &envs;
				break;
			case 'A':
				if (cap_parse_arg(&data.caps, optarg, CAP_SET) < 0)
					return EXIT_FAILURE;
				break;
			case 'D':
				if (cap_parse_arg(&data.caps, optarg, CAP_CLEAR) < 0)
					return EXIT_FAILURE;
				break;
			case 'P':
				apid = optarg;
				break;
			case 'u':
				data.unshare_flags |= CLONE_NEWUSER;
				data.remap_uid = strtoul(optarg, NULL, 10);
				break;
			case 'g':
				data.unshare_flags |= CLONE_NEWUSER;
				data.remap_gid = strtoul(optarg, NULL, 10);
				break;
			case 'U':
				parse_unshare_flags(&data.unshare_flags, optarg);
				break;
			case 'm':
				data.unshare_flags |= CLONE_NEWNS;
				data.mounts = parse_fstab(optarg);
				break;
			case 'v':
				verbose = 1;
				break;
			case 'V':
				print_version_and_exit();
				break;
			case '?':
				usage(EXIT_FAILURE);
		}
	}

	if (argc == optind) {
		error(0, 0, "New root directory required");
		usage(EXIT_FAILURE);
	}

	data.root = argv[optind++];

	if (access(data.root, R_OK | X_OK) < 0)
		error(EXIT_FAILURE, errno, "access: %s", data.root);

	if (argc == optind)
		error(EXIT_FAILURE, 0, "Command required");

	data.argv = &argv[optind];

	if (setgroups((size_t) 0, NULL) == -1)
		error(EXIT_FAILURE, errno, "setgroups");

	sanitize_fds();

	if (pipe2(pipe_1, O_DIRECT | O_CLOEXEC) < 0)
		error(EXIT_FAILURE, errno, "pipe2(1)");

	if (pipe2(pipe_2, O_DIRECT | O_CLOEXEC) < 0)
		error(EXIT_FAILURE, errno, "pipe2(2)");

	if ((pid = fork()) < 0)
		error(EXIT_FAILURE, errno, "fork");

	if (pid > 0) {
		close(pipe_2[1]);
		close(pipe_1[0]);

		if (apid)
			append_pid(apid, pid);

		return container_parent(pid, pipe_2[0], pipe_1[1], &data);
	}

	if (unshare(data.unshare_flags & ~CLONE_NEWUSER) < 0)
		error(EXIT_FAILURE, errno, "unshare");

	if (verbose)
		unshare_print_flags(data.unshare_flags);

	if ((pid = fork()) < 0)
		error(EXIT_FAILURE, errno, "fork");

	if (pid > 0) {
		int status = 0;
		pid_t cpid = 0;

		if (prctl(PR_SET_PDEATHSIG, SIGKILL) < 0)
			error(EXIT_FAILURE, errno, "prctl(PR_SET_PDEATHSIG)");

		if (write(pipe_2[1], &pid, sizeof(pid)) != sizeof(pid))
			error(EXIT_FAILURE, errno, "transfer pid");
		fsync(pipe_2[1]);

		sanitize_fds();
		free_data(&data);

		while (pid != cpid)
			if ((cpid = waitpid(-1, &status, 0)) < 0)
				error(EXIT_FAILURE, errno, "waitpid");

		return WEXITSTATUS(status);
	}

	close(pipe_2[0]);
	close(pipe_1[1]);

	return conatainer_child(&data, pipe_1[0], pipe_2[1]);
}
