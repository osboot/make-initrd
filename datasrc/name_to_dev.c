#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <alloca.h>
#include <inttypes.h>

#include "do_mounts.h"
#include "kinit.h"

#define BUF_SZ		65536

/* Find dev_t for e.g. "hda,NULL" or "hdb,2" */
static dev_t try_name(char *name, unsigned long int part)
{
	char path[BUF_SZ];
	char buf[BUF_SZ];
	unsigned long int major_num, minor_num, range;
	dev_t res;
	char *s;
	ssize_t len;
	int fd;

	/* read device number from /sys/block/.../dev */
	snprintf(path, sizeof(path), "/sys/block/%s/dev", name);
	fd = open(path, 0, 0);
	if (fd < 0)
		goto fail;
	len = read(fd, buf, BUF_SZ);
	close(fd);

	if (len <= 0 || len == BUF_SZ || buf[len - 1] != '\n')
		goto fail;
	buf[len - 1] = '\0';
	major_num = strtoul(buf, &s, 10);
	if (*s != ':')
		goto fail;
	minor_num = strtoul(s + 1, &s, 10);
	if (*s)
		goto fail;
	res = makedev((unsigned int) major_num, (unsigned int) minor_num);

	/* if it's there and we are not looking for a partition - that's it */
	if (!part)
		return res;

	/* otherwise read range from .../range */
	snprintf(path, sizeof(path), "/sys/block/%s/range", name);
	fd = open(path, 0, 0);
	if (fd < 0)
		goto fail;
	len = read(fd, buf, 32);
	close(fd);
	if (len <= 0 || len == 32 || buf[len - 1] != '\n')
		goto fail;
	buf[len - 1] = '\0';
	range = strtoul(buf, &s, 10);
	if (*s)
		goto fail;

	/* if partition is within range - we got it */
	if (part < range) {
		dprintf("kinit: try_name %s,%d = %s\n", name, part,
			bdevname(res + part));
		return res + part;
	}

fail:
	return (dev_t) 0;
}

/*
 *	Convert a name into device number.  We accept the following variants:
 *
 *	1) device number in hexadecimal	represents itself
 *	2) device number in major:minor decimal represents itself
 *	3) /dev/nfs represents Root_NFS
 *	4) /dev/<disk_name> represents the device number of disk
 *	5) /dev/<disk_name><decimal> represents the device number
 *         of partition - device number of disk plus the partition number
 *	6) /dev/<disk_name>p<decimal> - same as the above, that form is
 *	   used when disk name of partitioned disk ends on a digit.
 *	7) an actual block device node in the initramfs filesystem
 *
 *	If name doesn't have fall into the categories above, we return 0.
 *	Driverfs is used to check if something is a disk name - it has
 *	all known disks under bus/block/devices.  If the disk name
 *	contains slashes, name of driverfs node has them replaced with
 *	dots.  try_name() does the actual checks, assuming that driverfs
 *	is mounted on rootfs /sys.
 */

static inline dev_t name_to_dev_t_real(const char *name)
{
	char *p;
	dev_t res = 0;
	char *s;
	struct stat st;
	size_t part, len;
	const char *devname;
	char *cptr, *e1, *e2;
	unsigned long int major_num, minor_num;

	/* Are we a multi root line? */
	if (strchr(name, ','))
		return Root_MULTI;

	if (name[0] == '/') {
		devname = name;
	} else {
		char *dname = alloca(strlen(name) + 6);
		sprintf(dname, "/dev/%s", name);
		devname = dname;
	}

	if (!stat(devname, &st) && S_ISBLK(st.st_mode))
		return st.st_rdev;

	if (strncmp(name, "/dev/", 5)) {
		cptr = strchr(devname+5, ':');
		if (cptr && cptr[1] != '\0') {
			/* Colon-separated decimal device number */
			*cptr = '\0';
			major_num = strtoul(devname+5, &e1, 10);
			minor_num = strtoul(cptr+1, &e2, 10);
			if (!*e1 && !*e2)
				return makedev((unsigned int) major_num, (unsigned int) minor_num);
			*cptr = ':';
		} else {
			/* Hexadecimal device number */
			res = (dev_t) strtoul(name, &p, 16);
			if (!*p)
				return res;
		}
	} else {
		name += 5;
	}

	if (!strcmp(name, "nfs"))
		return Root_NFS;

	if (!strcmp(name, "ram")) /* /dev/ram - historic alias for /dev/ram0 */
		return Root_RAM0;

	if (!strncmp(name, "mtd", 3))
		return Root_MTD;

	len = strlen(name);
	s = alloca(len + 1);
	memcpy(s, name, len + 1);

	for (p = s; *p; p++)
		if (*p == '/')
			*p = '!';
	res = try_name(s, 0);
	if (res)
		return res;

	while (p > s && isdigit(p[-1]))
		p--;
	if (p == s || !*p || *p == '0')
		goto fail;
	part = strtoul(p, NULL, 10);
	*p = '\0';
	res = try_name(s, part);
	if (res)
		return res;

	if (p < s + 2 || !isdigit(p[-2]) || p[-1] != 'p')
		goto fail;
	p[-1] = '\0';
	res = try_name(s, part);
	return res;

fail:
	return (dev_t) 0;
}

dev_t name_to_dev_t(const char *name)
{
	dev_t dev = name_to_dev_t_real(name);

	dprintf("kinit: name_to_dev_t(%s) = %s\n", name, bdevname(dev));
	return dev;
}

#ifdef TEST_NAMETODEV		/* Standalone test */

int main(int argc, char *argv[])
{
	int i;

	for (i = 1; i < argc; i++)
		name_to_dev_t(argv[i]);

	return 0;
}

#endif
