// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * hddown.c	Find all disks on the system and
 *		shut them down.
 *
 * Copyright (C) 2003 Miquel van Smoorenburg.
 */
const char *v_hddown = "@(#)hddown.c  1.02  22-Apr-2003  miquels@cistron.nl";

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <libgen.h>

#include <sys/ioctl.h>
#include <linux/hdreg.h>
#include <linux/fs.h>

/*
 * sysfs part	Find all disks on the system, list out IDE and unmanaged
 *		SATA disks, flush the cache of those and shut them down.
 * Author:	Werner Fink <werner@suse.de>, 2007/06/12
 *
 */
#include <limits.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#ifdef WORDS_BIGENDIAN
#include <byteswap.h>
#endif

#include "reboot.h"

#define SYS_BLK		"/sys/block"
#define SYS_CLASS	"/sys/class/scsi_disk"
#define DEV_BASE	"/dev"
#define ISSPACE(c)	(((c)==' ')||((c)=='\n')||((c)=='\t')||((c)=='\v')||((c)=='\r')||((c)=='\f'))

/* Used in flush_cache_ext(), compare with <linux/hdreg.h> */
#define IDBYTES		512
#define MASK_EXT	0xE000		/* Bit 15 shall be zero, bit 14 shall be one, bit 13 flush cache ext */
#define TEST_EXT	0x6000

/* Maybe set in list_disks() and used in do_standby_disk() */
#define DISK_IS_IDE	0x00000001
#define DISK_IS_SATA	0x00000002
#define DISK_EXTFLUSH	0x00000004
#define DISK_REMOVABLE	0x00000008
#define DISK_MANAGED	0x00000010
#define DISK_FLUSHONLY	0x00000020

static char *strstrip(char *str);
static FILE *hdopen(const char *const format, const char *const name);
static int flush_cache_ext(const char *device);

/*
 *	Find all disks through /sys/block.
 */
static char *list_disks(DIR *blk, unsigned int *flags)
{
	struct dirent *d;

	while ((d = readdir(blk))) {
		(*flags) = 0;
		if (d->d_name[1] == 'd' && (d->d_name[0] == 'h' || d->d_name[0] == 's')) {
			char buf[NAME_MAX+1], lnk[NAME_MAX+1], *ptr;
			FILE *fp;
			ssize_t ret;

			fp = hdopen(SYS_BLK "/%s/removable", d->d_name);
			if ((long)fp <= 0) {
				if ((long)fp < 0)
					goto empty;	/* error */
				continue;		/* no entry `removable' */
			}

			ret = getc(fp);
			fclose(fp);

			if (ret != '0')
				(*flags) |= DISK_REMOVABLE;

			if (d->d_name[0] == 'h') {
				if ((*flags) & DISK_REMOVABLE)
					continue;	/* not a hard disk */

				(*flags) |= DISK_IS_IDE;
				if ((ret = flush_cache_ext(d->d_name))) {
					if (ret < 0)
						goto empty;
					(*flags) |= DISK_EXTFLUSH;
				}
				break;			/* old IDE disk not managed by kernel, out here */
			}

			ret = snprintf(buf, sizeof(buf), SYS_BLK "/%s/device", d->d_name);
			if ((ret >= (int)sizeof(buf)) || (ret < 0))
				goto empty;		/* error */

			ret = readlink(buf, lnk, sizeof(lnk));
			if (ret >= (int)sizeof(lnk))
				goto empty;		/* error */
			if (ret < 0) {
				if (errno != ENOENT)
					goto empty;	/* error */
				continue;		/* no entry `device' */
			}
			lnk[ret] = '\0';

			ptr = basename(lnk);
			if (!ptr || !*ptr)
				continue;		/* should not happen */

			fp = hdopen(SYS_CLASS "/%s/manage_start_stop", ptr);
			if ((long)fp <= 0) {
				if ((long)fp < 0)
					goto empty;	/* error */
			} else {
				ret = getc(fp);
				fclose(fp);

				if (ret != '0') {
					(*flags) |= DISK_MANAGED;
					continue;
				}
			}

			fp = hdopen(SYS_BLK "/%s/device/vendor", d->d_name);
			if ((long)fp <= 0) {
				if ((long)fp < 0)
					goto empty;	/* error */
				continue;		/* no entry `device/vendor' */
			}

			ptr = fgets(buf, sizeof(buf), fp);
			fclose(fp);
			if (ptr == (char *)0)
				continue;		/* should not happen */

			ptr = strstrip(buf);
			if (*ptr == '\0')
				continue;		/* should not happen */

			if (strncmp(buf, "ATA", sizeof(buf)) == 0) {
				if ((*flags) & DISK_REMOVABLE)
					continue;	/* not a hard disk */

				(*flags) |= (DISK_IS_IDE|DISK_IS_SATA);
				if ((ret = flush_cache_ext(d->d_name))) {
					if (ret < 0)
						goto empty;
					(*flags) |= DISK_EXTFLUSH;
				}
				break;			/* new SATA disk to shutdown, out here */
			}

			if (((*flags) & DISK_REMOVABLE) == 0)
				continue;		/* Seems to be a real SCSI disk */

			if ((ret = flush_cache_ext(d->d_name))) {
				if (ret < 0)
					goto empty;
				(*flags) |= DISK_EXTFLUSH;
			}
			break;				/* Removable disk like USB stick to shutdown */
		}
	}
	if (d == (struct dirent *)0)
		goto empty;
	return d->d_name;
empty:
	return (char *)0;
}

/*
 *	Put an IDE/SCSI/SATA disk in standby mode.
 *	Code stolen from hdparm.c
 */
static int do_standby_disk(char *device, unsigned int flags)
{
#ifndef WIN_STANDBYNOW1
#define WIN_STANDBYNOW1		0xE0
#endif
#ifndef WIN_STANDBYNOW2
#define WIN_STANDBYNOW2		0x94
#endif
#ifndef WIN_FLUSH_CACHE_EXT
#define WIN_FLUSH_CACHE_EXT	0xEA
#endif
#ifndef WIN_FLUSH_CACHE
#define WIN_FLUSH_CACHE		0xE7
#endif
	unsigned char flush1[4] = {WIN_FLUSH_CACHE_EXT,0,0,0};
	unsigned char flush2[4] = {WIN_FLUSH_CACHE,0,0,0};
	unsigned char stdby1[4] = {WIN_STANDBYNOW1,0,0,0};
	unsigned char stdby2[4] = {WIN_STANDBYNOW2,0,0,0};
	char buf[NAME_MAX+1];
	int fd, ret;

	ret = snprintf(buf, sizeof(buf), DEV_BASE "/%s", device);
	if ((ret >= (int)sizeof(buf)) || (ret < 0))
		return -1;

	if ((fd = open(buf, O_RDWR|O_NONBLOCK)) < 0)
		return -1;

	switch (flags & DISK_EXTFLUSH) {
		case DISK_EXTFLUSH:
			if ((ret = ioctl(fd, HDIO_DRIVE_CMD, &flush1)) == 0)
				break;
			/* Extend flush rejected, try standard flush */
			__attribute__ ((fallthrough));
		default:
			ret = ioctl(fd, HDIO_DRIVE_CMD, &flush2) &&
			      ioctl(fd, BLKFLSBUF);
			break;
	}

	if ((flags & DISK_FLUSHONLY) == 0x0) {
		ret = ioctl(fd, HDIO_DRIVE_CMD, &stdby1) &&
		      ioctl(fd, HDIO_DRIVE_CMD, &stdby2);
	}

	close(fd);

	if (ret)
		return -1;
	return 0;
}

/*
 *	List all disks and put them in standby mode.
 *	This has the side-effect of flushing the writecache,
 *	which is exactly what we want on poweroff.
 */
int hddown(void)
{
	unsigned int flags;
	char *disk;
	DIR *blk;

	if ((blk = opendir(SYS_BLK)) == (DIR *)0)
		return -1;

	while ((disk = list_disks(blk, &flags)))
		do_standby_disk(disk, flags);

	return closedir(blk);
}

/*
 *	List all disks and cause them to flush their buffers.
 */
int hdflush(void)
{
	unsigned int flags;
	char *disk;
	DIR *blk;

	if ((blk = opendir(SYS_BLK)) == (DIR *)0)
		return -1;

	while ((disk = list_disks(blk, &flags)))
		do_standby_disk(disk, (flags|DISK_FLUSHONLY));

	return closedir(blk);
}

/*
 * Strip off trailing white spaces
 */
static char *strstrip(char *str)
{
	const size_t len = strlen(str);
	if (len) {
		char *end = str + len - 1;
		while ((end != str) && ISSPACE(*end))
			end--;
		*(end + 1) = '\0';			/* remove trailing white spaces */
	}
	return str;
}

/*
 * Open a sysfs file without getting a controlling tty
 * and return FILE* pointer.
 */
static FILE *hdopen(const char *const format, const char *const name)
{
	char buf[NAME_MAX+1];
	FILE *fp = (FILE *)-1;
	int fd, ret;

	ret = snprintf(buf, sizeof(buf), format, name);
	if ((ret >= (int)sizeof(buf)) || (ret < 0))
		goto error;		/* error */

	fd = open(buf, O_RDONLY|O_NOCTTY);
	if (fd < 0) {
		if (errno != ENOENT)
			goto error;	/* error */
		fp = (FILE *)0;
		goto error;		/* no entry `removable' */
	}

	fp = fdopen(fd, "r");
	if (fp == (FILE *)0)
		close(fd);		/* should not happen */
error:
	return fp;
}

/*
 * Check IDE/(S)ATA hard disk identity for
 * the FLUSH CACHE EXT bit set.
 */
static int flush_cache_ext(const char *device)
{
#ifndef WIN_IDENTIFY
#define WIN_IDENTIFY		0xEC
#endif
	unsigned char args[4+IDBYTES];
	unsigned short *id = (unsigned short *)(&args[4]);
	char buf[NAME_MAX+1], *ptr;
	int fd = -1, ret = 0;
	FILE *fp;

	fp = hdopen(SYS_BLK "/%s/size", device);
	if ((long)fp <= 0) {
		if ((long)fp < 0)
			return -1;	/* error */
		goto out;		/* no entry `size' */
	}

	ptr = fgets(buf, sizeof(buf), fp);
	fclose(fp);
	if (ptr == (char *)0)
		goto out;		/* should not happen */

	ptr = strstrip(buf);
	if (*ptr == '\0')
		goto out;		/* should not happen */

	if ((size_t)atoll(buf) < (1<<28))
		goto out;		/* small disk */

	ret = snprintf(buf, sizeof(buf), DEV_BASE "/%s", device);
	if ((ret >= (int)sizeof(buf)) || (ret < 0))
		return -1;		/* error */

	if ((fd = open(buf, O_RDONLY|O_NONBLOCK)) < 0)
		goto out;

	memset(&args[0], 0, sizeof(args));
	args[0] = WIN_IDENTIFY;
	args[3] = 1;
	if (ioctl(fd, HDIO_DRIVE_CMD, &args))
		goto out;
#ifdef WORDS_BIGENDIAN
# if 0
	{
		const unsigned short *end = id + IDBYTES/2;
		const unsigned short *from = id;
		unsigned short *to = id;

		while (from < end)
			*to++ = bswap_16(*from++);
	}
# else
	id[83] = bswap_16(id[83]);
# endif
#endif
	if ((id[83] & MASK_EXT) == TEST_EXT)
		ret = 1;
out:
	if (fd >= 0)
		close(fd);
	return ret;
}
