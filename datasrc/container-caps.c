#include <sys/capability.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <error.h>

#include "container.h"

static struct {
	const char *name;
	const cap_flag_t value;
} const cflags[] = {
	{ "effective", CAP_EFFECTIVE },
	{ "inheritable", CAP_INHERITABLE },
	{ "permitted", CAP_PERMITTED },
	{ NULL, 0 }
};

int
cap_change_flag(cap_t caps, const char *capname, const cap_flag_value_t value)
{
	int i;
	cap_value_t cap;

	if (!strncasecmp("all", capname, 3)) {
		if (value == CAP_CLEAR)
			return cap_clear(caps);

		for (cap = CAP_LAST_CAP; cap >= 0; cap--) {
			for (i = 0; cflags[i].name; i++) {
				if (!cap_set_flag(caps, cflags[i].value, 1, &cap, CAP_SET))
					continue;

				error(EXIT_SUCCESS, 0, "unable to SET capability (%s): %s",
				      cflags[i].name, cap_to_name(cap));
			}
		}

		return 0;
	}

	if (cap_from_name(capname, &cap) < 0) {
		error(EXIT_SUCCESS, 0, "unknown capability: %s", capname);
		return -1;
	}

	for (i = 0; cflags[i].name; i++) {
		if (!cap_set_flag(caps, cflags[i].value, 1, &cap, value))
			continue;

		error(EXIT_SUCCESS, 0, "unable to %s capability (%s): %s",
		      (value == CAP_CLEAR ? "unset" : "set"), cflags[i].name, cap_to_name(cap));

		return -1;
	}

	return 0;
}

int
cap_parse_arg(cap_t *caps, char *arg, const cap_flag_value_t value)
{
	int i = 0;
	char *str, *token, *saveptr;

	if (!*caps)
		*caps = cap_get_proc();

	for (i = 1, str = arg;; i++, str = NULL) {
		if (!(token = strtok_r(str, ",", &saveptr)))
			break;

		if (cap_change_flag(*caps, token, value) < 0)
			return -1;
	}

	return 0;
}

void
apply_caps(cap_t caps)
{
	if (!CAP_IS_SUPPORTED(CAP_SETFCAP))
		error(EXIT_FAILURE, 0, "the kernel does not support CAP_SETFCAP");

	if (cap_set_proc(caps) < 0)
		error(EXIT_FAILURE, errno, "cap_set_proc");

	if (cap_free(caps) < 0)
		error(EXIT_FAILURE, errno, "cap_free");
}
