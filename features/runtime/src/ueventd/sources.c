// SPDX-License-Identifier: GPL-3.0-or-later

#include "config.h"

#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <signal.h>
#include <time.h>
#include <errno.h>

#include "cleanup.h"
#include "rd/logging.h"
#include "rd/memory.h"
#include "ueventd.h"

#define SOURCE_RESET_UPTIME_MS  10000ULL
#define SOURCE_STOP_TIMEOUT_MS  500ULL

struct source_proc {
	struct source_proc *next;
	pid_t pid;
	uint64_t last_start_ms;
	uint64_t restart_at_ms;
	unsigned int restarts;
	char path[];
};

static struct source_proc *source_list = NULL;

static uint64_t boottime_ms(void)
{
	struct timespec ts = { 0 };

	clock_gettime(CLOCK_BOOTTIME, &ts);
	return (uint64_t) ts.tv_sec * 1000ULL + (uint64_t) ts.tv_nsec / 1000000ULL;
}

static uint64_t restart_backoff_ms(unsigned int restarts)
{
	static const uint64_t delays[] = { 200, 500, 1000, 2000, 5000 };
	size_t idx = (restarts > 0) ? (size_t) (restarts - 1) : 0;

	if (idx >= ARRAY_SIZE(delays))
		idx = ARRAY_SIZE(delays) - 1;

	return delays[idx];
}

static void spawn_source(struct source_proc *src)
{
	pid_t pid = fork();

	if (pid < 0) {
		rd_err("fork(%s): %m", src->path);
		src->restarts++;
		src->restart_at_ms = boottime_ms() + restart_backoff_ms(src->restarts);
		return;
	}

	if (pid > 0) {
		src->pid = pid;
		src->last_start_ms = boottime_ms();
		src->restart_at_ms = 0;
		rd_info("started source %s (pid=%d)", src->path, (int) pid);
		return;
	}

	if (prctl(PR_SET_PDEATHSIG, SIGTERM) < 0)
		rd_fatal("prctl(PR_SET_PDEATHSIG): %m");

	sigset_t mask;
	sigemptyset(&mask);
	if (sigprocmask(SIG_SETMASK, &mask, NULL) < 0)
		rd_fatal("sigprocmask: %m");

	execl(src->path, src->path, (char *) NULL);
	rd_fatal("exec(%s): %m", src->path);
}

void sources_start(const char *dir_path)
{
	struct dirent *ent;

	if (!dir_path || !*dir_path)
		return;

	DIR *d _free_(closedir) = opendir(dir_path);
	if (!d) {
		if (errno != ENOENT)
			rd_err("opendir: %s: %m", dir_path);
		return;
	}

	while ((ent = xreaddir(d, dir_path)) != NULL) {
		struct stat st;

		if (strncmp(ent->d_name, "ueventd-", 8) != 0)
			continue;

		char *fullpath _free_(free) = rd_asprintf_or_die("%s/%s", dir_path, ent->d_name);

		if (stat(fullpath, &st) < 0) {
			rd_err("stat: %s: %m", fullpath);
			continue;
		}

		if (!S_ISREG(st.st_mode) || !(st.st_mode & S_IXUSR))
			continue;

		size_t plen = strlen(fullpath) + 1;
		struct source_proc *src = rd_calloc_or_die(1, sizeof(*src) + plen);
		memcpy(src->path, fullpath, plen);

		src->next = source_list;
		source_list = src;

		spawn_source(src);
	}
}

int sources_handle_exit(pid_t pid, int wstatus)
{
	struct source_proc *src;
	uint64_t now;
	uint64_t delay;

	for (src = source_list; src; src = src->next) {
		if (src->pid != pid)
			continue;

		src->pid = 0;
		now = boottime_ms();

		if (now >= src->last_start_ms &&
		    (now - src->last_start_ms) >= SOURCE_RESET_UPTIME_MS)
			src->restarts = 0;

		src->restarts++;
		delay = restart_backoff_ms(src->restarts);
		src->restart_at_ms = now + delay;

		if (WIFEXITED(wstatus)) {
			rd_err("source %s (pid=%d) exited with status %d, restarting in %llums",
			       src->path, (int) pid, WEXITSTATUS(wstatus),
			       (unsigned long long) delay);
		} else if (WIFSIGNALED(wstatus)) {
			rd_err("source %s (pid=%d) killed by signal %d, restarting in %llums",
			       src->path, (int) pid, WTERMSIG(wstatus),
			       (unsigned long long) delay);
		}

		return 1;
	}

	return 0;
}

int sources_next_timeout_ms(void)
{
	struct source_proc *src;
	uint64_t now = 0;
	int min_ms = -1;

	for (src = source_list; src; src = src->next) {
		if (src->pid != 0 || src->restart_at_ms == 0)
			continue;

		if (!now)
			now = boottime_ms();

		int rem = (src->restart_at_ms > now)
		          ? (int) (src->restart_at_ms - now)
		          : 0;

		if (min_ms < 0 || rem < min_ms)
			min_ms = rem;
	}

	return min_ms;
}

void sources_tick(void)
{
	struct source_proc *src;
	uint64_t now = 0;

	for (src = source_list; src; src = src->next) {
		if (src->pid != 0 || src->restart_at_ms == 0)
			continue;

		if (!now)
			now = boottime_ms();

		if (now >= src->restart_at_ms)
			spawn_source(src);
	}
}

void sources_stop_all(void)
{
	struct source_proc *src, *next;
	int alive = 0;

	for (src = source_list; src; src = src->next) {
		if (src->pid > 0) {
			kill(src->pid, SIGTERM);
			alive++;
		}
	}

	if (alive > 0) {
		uint64_t deadline = boottime_ms() + SOURCE_STOP_TIMEOUT_MS;

		while (alive > 0 && boottime_ms() < deadline) {
			for (src = source_list; src; src = src->next) {
				if (src->pid <= 0)
					continue;
				if (waitpid_retry(src->pid, NULL, WNOHANG) > 0) {
					src->pid = 0;
					alive--;
				}
			}
			if (alive > 0) {
				struct timespec ts = { .tv_sec = 0, .tv_nsec = 20000000L };
				nanosleep(&ts, NULL);
			}
		}

		for (src = source_list; src; src = src->next) {
			if (src->pid > 0) {
				kill(src->pid, SIGKILL);
				waitpid_retry(src->pid, NULL, 0);
				src->pid = 0;
			}
		}
	}

	for (src = source_list; src; src = next) {
		next = src->next;
		free(src);
	}
	source_list = NULL;
}
