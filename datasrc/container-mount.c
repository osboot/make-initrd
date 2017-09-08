#include <sys/mount.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <error.h>
#include <mntent.h>

#include <dirent.h>
#include <fcntl.h>

#include "container.h"

#define MNTBUFSIZ 1024

extern int verbose;

static const char *const mountflag_names[] = {
	"ro", "rw",                     // MS_RDONLY
	"noatime", "atime",             // MS_NOATIME
	"nodev", "dev",                 // MS_NODEV
	"nodiratime", "diratime",       // MS_NODIRATIME
	"noexec", "exec",               // MS_NOEXEC
	"nosuid", "suid",               // MS_NOSUID
	"sync", "async",                // MS_SYNCHRONOUS
	"relatime", "norelatime",       // MS_RELATIME
	"strictatime", "nostrictatime", // MS_STRICTATIME
	"dirsync", "nodirsync",         // MS_DIRSYNC
	"lazytime", "nolazytime",       // MS_LAZYTIME
	"mand", "nomand",               // MS_MANDLOCK
	"silent", "loud",               // ignore
	"defaults", "nodefaults",       // ignore
	"auto", "noauto",               // ignore
	"comment",                      // ignore, systemd uses this in fstab
	"_netdev",                      // ignore
	"loop",                         // ignore
	"rec",                          // MS_REC
	"bind",                         // MS_BIND
	"rbind",                        // MS_BIND | MS_REC
	"move",                         // MS_MOVE
	"remount",                      // MS_REMOUNT
	"shared",                       // MS_SHARED
	"rshared",                      // MS_SHARED | MS_REC
	"slave",                        // MS_SLAVE
	"rslave",                       // MS_SLAVE | MS_REC
	NULL
};

#define E(x) \
	{ x, 0 }, { x, 1 }

static struct {
	const unsigned long id;
	const short invert;
} const mountflag_values[] = {
	E(MS_RDONLY),
	E(MS_NOATIME),
	E(MS_NODEV),
	E(MS_NODIRATIME),
	E(MS_NOEXEC),
	E(MS_NOSUID),
	E(MS_SYNCHRONOUS),
	E(MS_RELATIME),
	E(MS_STRICTATIME),
	E(MS_DIRSYNC),
	E(MS_LAZYTIME),
	E(MS_MANDLOCK),
	E(MS_SILENT),
	E(0),
	E(0),
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ MS_REC, 0 },
	{ MS_BIND, 0 },
	{ MS_BIND | MS_REC, 0 },
	{ MS_MOVE, 0 },
	{ MS_REMOUNT, 0 },
	{ MS_SHARED, 0 },
	{ MS_SHARED | MS_REC, 0 },
	{ MS_SLAVE, 0 },
	{ MS_SLAVE | MS_REC, 0 },
};

#undef E

struct mountflags {
	unsigned long vfs_opts;
	char *data;
	mode_t mkdir;
};

void
free_mntent(struct mntent *ent)
{
	xfree(ent->mnt_fsname);
	xfree(ent->mnt_dir);
	xfree(ent->mnt_type);
	xfree(ent->mnt_opts);
	xfree(ent);
}

static mode_t
str2umask(const char *name, const char *value)
{
	char *p = 0;
	unsigned long n;

	if (!*value)
		error(EXIT_FAILURE, 0, "empty value for \"%s\" option", name);

	n = strtoul(value, &p, 8);
	if (!p || *p || n > 0777)
		error(EXIT_FAILURE, 0, "invalid value for \"%s\" option: %s", name, value);

	return (mode_t) n;
}

static struct mountflags *
parse_mountopts(const char *opts, struct mountflags *flags)
{
	char *s, *subopts, *value;
	int i;

	flags->vfs_opts = 0;
	flags->data     = NULL;

	s = subopts = xstrdup(opts);

	while (*subopts != '\0') {
		value = NULL;

		if ((i = getsubopt(&subopts, (char **) mountflag_names, &value)) < 0) {
			if (!strncasecmp("x-mount.mkdir", value, 13)) {
				flags->mkdir = (!strncasecmp("x-mount.mkdir=", value, 14))
				                   ? str2umask("x-mount.mkdir", value + 14)
				                   : 0755;
				continue;
			}

			if (!strncasecmp("x-", value, 2))
				continue;

			if (flags->data) {
				size_t datalen = strlen(flags->data) + strlen(value) + 1;
				flags->data    = xrealloc(flags->data, datalen, 1);
				snprintf(flags->data, datalen, "%s,%s", flags->data, value);
			} else {
				flags->data = xstrdup(value);
			}
			continue;
		}

		if (mountflag_values[i].invert)
			flags->vfs_opts &= ~mountflag_values[i].id;
		else
			flags->vfs_opts |= mountflag_values[i].id;
	}

	xfree(s);
	return flags;
}

struct mntent **
parse_fstab(const char *fstabname)
{
	FILE *fstab;
	struct mntent mt;
	char *buf;

	struct mntent **result = NULL;
	size_t n_ents          = 0;

	fstab = setmntent(fstabname, "r");
	if (!fstab)
		error(EXIT_FAILURE, errno, "setmntent: %s", fstabname);

	buf = xmalloc(MNTBUFSIZ);

	while (getmntent_r(fstab, &mt, buf, MNTBUFSIZ)) {
		result = xrealloc(result, (n_ents + 1), sizeof(void *));

		result[n_ents]             = xcalloc(1, sizeof(struct mntent));
		result[n_ents]->mnt_fsname = xstrdup(mt.mnt_fsname);
		result[n_ents]->mnt_dir    = xstrdup(mt.mnt_dir);
		result[n_ents]->mnt_type   = xstrdup(mt.mnt_type);
		result[n_ents]->mnt_opts   = xstrdup(mt.mnt_opts);
		result[n_ents]->mnt_freq   = mt.mnt_freq;
		result[n_ents]->mnt_passno = mt.mnt_passno;

		n_ents++;
	}

	result         = xrealloc(result, (n_ents + 1), sizeof(void *));
	result[n_ents] = NULL;

	endmntent(fstab);
	xfree(buf);

	return result;
}

static int
_bindents(const char *source, const char *target)
{
	int rc = -1;
	DIR *d;
	char *spath, *tpath;

	if (!(d = opendir(source))) {
		error(EXIT_SUCCESS, errno, "opendir: %s", source);
		return rc;
	}

	spath = tpath = NULL;

	while (1) {
		struct dirent *ent;

		if (!(ent = readdir(d))) {
			if (errno) {
				error(EXIT_SUCCESS, errno, "readdir: %s", source);
				break;
			}
			rc = 0;
			break;
		}

		if (!strcmp(".", ent->d_name) || !strcmp("..", ent->d_name))
			continue;

		xasprintf(&spath, "%s/%s", source, ent->d_name);
		xasprintf(&tpath, "%s/%s", target, ent->d_name);

		if (ent->d_type == DT_DIR) {
			if (mkdir(tpath, 0755) < 0) {
				error(EXIT_SUCCESS, errno, "mkdir: %s", tpath);
				break;
			}
		} else {
			int fd = creat(tpath, 0644);
			if (fd < 0) {
				error(EXIT_SUCCESS, errno, "open: %s", tpath);
				break;
			}
			close(fd);
		}

		if (mount(spath, tpath, "none", MS_BIND | MS_REC, NULL) < 0) {
			error(EXIT_SUCCESS, errno, "mount: %s", tpath);
			break;
		}

		spath = xfree(spath);
		tpath = xfree(tpath);
	}

	xfree(spath);
	xfree(tpath);

	if (closedir(d) < 0) {
		error(EXIT_SUCCESS, errno, "closedir: %s", source);
		return -1;
	}

	return rc;
}

void
do_mount(const char *newroot, struct mntent **mounts)
{
	size_t i = 0;

	while (mounts && mounts[i]) {
		char *mpoint;
		struct mountflags mflags = {};

		parse_mountopts(mounts[i]->mnt_opts, &mflags);

		if ((strlen(newroot) + strlen(mounts[i]->mnt_dir)) > MAXPATHLEN)
			error(EXIT_FAILURE, 0, "mountpoint name too long");

		xasprintf(&mpoint, "%s%s", newroot, mounts[i]->mnt_dir);

		if (mflags.mkdir && mkdir(mpoint, mflags.mkdir) < 0 && errno != EEXIST)
			error(EXIT_FAILURE, errno, "mkdir: %s", mpoint);

		if (access(mpoint, F_OK) < 0) {
			if (verbose)
				dprintf(STDERR_FILENO, "WARNING: mountpoint not found in the container: %s\n", mounts[i]->mnt_dir);
			goto next;
		}

		if (!strncasecmp("_bindents", mounts[i]->mnt_type, 9)) {
			if (verbose)
				dprintf(STDERR_FILENO, "mount(bind) content into the container: %s\n", mpoint);

			if (mount("tmpfs", mpoint, "tmpfs", mflags.vfs_opts, mflags.data) < 0)
				error(EXIT_FAILURE, errno, "mount(_bindents): %s", mpoint);

			if (_bindents(mounts[i]->mnt_fsname, mpoint) < 0)
				error(EXIT_FAILURE, 0, "_bindents: %s", mpoint);

			goto next;
		}

		if (!strncasecmp("_umount", mounts[i]->mnt_type, 7)) {
			if (verbose)
				dprintf(STDERR_FILENO, "umount from the container: %s\n", mpoint);

			if (umount2(mpoint, MNT_DETACH) < 0)
				error(EXIT_FAILURE, errno, "umount2: %s", mpoint);

			goto next;
		}

		if (verbose) {
			if (mflags.vfs_opts & MS_BIND)
				dprintf(STDERR_FILENO, "mount(bind) into the container: %s\n", mpoint);
			else if (mflags.vfs_opts & MS_MOVE)
				dprintf(STDERR_FILENO, "mount(move) into the container: %s\n", mpoint);
			else
				dprintf(STDERR_FILENO, "mount into the container: %s\n", mpoint);
		}

		if (mount(mounts[i]->mnt_fsname, mpoint, mounts[i]->mnt_type, mflags.vfs_opts, mflags.data) < 0)
			error(EXIT_FAILURE, errno, "mount: %s", mpoint);
	next:

		xfree(mflags.data);
		xfree(mpoint);

		free_mntent(mounts[i]);
		i++;
	}
}
