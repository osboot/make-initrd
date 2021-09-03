#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <linux/kd.h>
#include <linux/keyboard.h> /* KG_* */
#include <string.h>    /* strcmp */
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>    /* system */
#include <fcntl.h>     /* open */
#include <sys/ioctl.h> /* ioctl */
#include <unistd.h>    /* sleep */
#include <err.h>       /* err */

struct modifier {
	const char *name;
	const int bit;
} modifiers[] = {
	{ "plain",      0 },
	{ "shift",     (1 << KG_SHIFT)  },
	{ "altgr",     (1 << KG_ALTGR)  },
	{ "control",   (1 << KG_CTRL)   },
	{ "alt",       (1 << KG_ALT)    },
	{ "shiftl",    (1 << KG_SHIFTL) },
	{ "shiftr",    (1 << KG_SHIFTR) },
	{ "ctrl",      (1 << KG_CTRLL)  }, // alias
	{ "ctrll",     (1 << KG_CTRLL)  },
	{ "ctrlr",     (1 << KG_CTRLR)  },
	{ "capsshift", (1 << KG_CAPSSHIFT) },
	{ 0, 0 }
};

static const char *conspath[] = {
	"/dev/console",
	"/dev/systty",
	"/dev/tty0",
	"/dev/tty",
	"/dev/vc/0",
	"/proc/self/fd/0",
	NULL
};

int confd;
int verbose = 0;
char *prog;

static void show_usage(int rc)
{
	fprintf(stderr, "Usage: %s [-v|-h] [modificators ...] keycode KEYCODE\n", prog);
	exit(rc);
}

static int is_a_console(int fd)
{
	char arg = 0;
	return (isatty(fd) &&
	        !ioctl(fd, KDGKBTYPE, &arg) &&
	        ((arg == KB_101) || (arg == KB_84)));
}

static int open_a_console(const char *fnam)
{
	int fd;

	/*
	 * For ioctl purposes we only need some fd and permissions
	 * do not matter.
	 */
	fd = open(fnam, O_RDWR);
	if (fd < 0)
		fd = open(fnam, O_WRONLY);
	if (fd < 0)
		fd = open(fnam, O_RDONLY);
	if (fd < 0)
		return -1;
	return fd;
}

static int getfd(void)
{
	int fd;

	for (int i = 0; conspath[i]; i++) {
		if ((fd = open_a_console(conspath[i])) >= 0) {
			if (is_a_console(fd)) {
				if (verbose)
					warnx("using %s", conspath[i]);
				return fd;
			}
			close(fd);
		}
	}

	for (fd = 0; fd < 3; fd++) {
		if (is_a_console(fd)) {
			if (verbose)
				warnx("using %d", fd);
			return fd;
		}
	}

	/* total failure */
	errx(EXIT_FAILURE, "couldn't get a file descriptor referring to the console.");
}

static void sighup(int n __attribute__((unused)))
{
	if (verbose > 1)
		warnx("spawing shell");
	if (system("/lib/initrd/spawn-shell") < 0)
		warn("system");

	signal(SIGHUP, sighup);
}

int main(int argc, char **argv)
{
	struct kbentry ke;
	int opt, mod = 0;
	int keycode = -1;

	prog = strrchr(argv[0], '/');
	if (!prog)
		prog = argv[0];
	else
		prog++;

	while ((opt = getopt(argc, argv, "vh")) != -1) {
		switch (opt) {
			case 'v':
				verbose++;
				break;
			case 'h':
				show_usage(EXIT_SUCCESS);
				break;
			default: /* '?' */
				show_usage(EXIT_FAILURE);
		}
	}

	if (optind == argc)
		show_usage(EXIT_SUCCESS);

	confd = getfd();

	for (int i = optind; i < argc; i++) {
		if (!strcasecmp(argv[i], "keycode")) {
			i++;

			if (i == argc)
				errx(EXIT_FAILURE, "key number required");

			if ((i + 1) < argc)
				errx(EXIT_FAILURE, "too many arguments");

			keycode = atoi(argv[i]);
			break;
		}

		struct modifier *modifier = modifiers;
		bool found = false;

		while (modifier->name) {
			if (!strcasecmp(modifier->name, argv[i])) {
				mod |= modifier->bit;
				found = true;
				break;
			}
			modifier++;
		}

		if (!found)
			errx(EXIT_FAILURE, "unknown modifier: %s", argv[i]);
	}

	if (keycode < 0)
		errx(EXIT_FAILURE, "'keycode KEYCODE' required");

	ke.kb_table = (unsigned char) mod;
	ke.kb_index = (unsigned char) keycode;
	ke.kb_value = K_SPAWNCONSOLE;

	errno = 0;
	if (ioctl(confd, KDSKBENT, (unsigned long)&ke) < 0)
		err(EXIT_FAILURE, "ioctl KDSKBENT (errno=%d)", errno);

	signal(SIGHUP, sighup);

	errno = 0;
	if (ioctl(confd, KDSIGACCEPT, (long) SIGHUP) < 0)
		err(EXIT_FAILURE, "ioctl KDSIGACCEPT (errno=%d)", errno);

	while (1)
		sleep(3600);

	return EXIT_SUCCESS;
}
