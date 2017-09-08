#include <sys/stat.h>
#include <sys/sysmacros.h>

#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <error.h>

#include "container.h"

#define LINESIZ 256

void
make_devices(struct mapfile *devs)
{
	size_t i;
	char *nline, *a;
	char s[LINESIZ];

	i = 1;
	a = devs->map;
	while (a && a[0]) {
		char *path  = NULL;
		mode_t mode = 0;
		uid_t uid   = (uid_t) -1;
		gid_t gid   = (gid_t) -1;
		char type;
		unsigned int major, minor;

		nline = strchr(a, '\n');

		while (isspace(a[0]))
			a++;

		if (strncmp("nod", a, 3) || a[0] == '#') {
			a = (nline) ? nline + 1 : NULL;
			i++;
			continue;
		}

		if (!nline)
			nline = a + strlen(a);

		if ((nline - a) > LINESIZ)
			error(EXIT_FAILURE, 0, "%s:%lu: string too long", devs->filename, i);

		strncpy(s, a, (size_t)(nline - a));
		s[nline - a] = '\0';

		a = nline + 1;

		if (sscanf(s, "nod %ms %o %u %u %c %u %u", &path, &mode, &uid, &gid, &type, &major, &minor) != 7)
			error(EXIT_FAILURE, 0, "%s:%lu: bad line format", devs->filename, i);

		switch (type) {
			case 'c':
				mode |= S_IFCHR;
				break;
			case 'b':
				mode |= S_IFBLK;
				break;
			case 'p':
				mode |= S_IFIFO;
				break;
			case 's':
				mode |= S_IFSOCK;
				break;
			default:
				error(EXIT_FAILURE, 0, "%s:%lu: bad device type", devs->filename, i);
		}

		if (mknod(path, mode, makedev(major, minor)) < 0)
			error(EXIT_FAILURE, errno, "mknod: %s", path);

		if (lchown(path, uid, gid) < 0)
			error(EXIT_FAILURE, errno, "lchown: %s", path);

		path = xfree(path);
		i++;
	}

	close_map(devs);
}
