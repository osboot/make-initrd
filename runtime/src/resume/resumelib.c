/*
 * Handle resume from suspend-to-disk
 */

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>

#include "kinit.h"
#include "do_mounts.h"
#include "resume.h"

#ifndef CONFIG_PM_STD_PARTITION
# define CONFIG_PM_STD_PARTITION ""
#endif

int do_resume(int argc, char *argv[])
{
	const char *resume_file = CONFIG_PM_STD_PARTITION;
	const char *resume_arg;
	unsigned long long resume_offset;

	resume_arg = get_arg(argc, argv, "resume=");
	resume_file = resume_arg ? resume_arg : resume_file;
	/* No resume device specified */
	if (!resume_file[0])
		return 0;

	resume_arg = get_arg(argc, argv, "resume_offset=");
	resume_offset = resume_arg ? strtoull(resume_arg, NULL, 0) : 0ULL;

	/* Fix: we either should consider reverting the device back to
	   ordinary swap, or (better) put that code into swapon */
	/* Noresume requested */
	if (get_flag(argc, argv, "noresume"))
		return 0;
	return resume(resume_file, resume_offset);
}

int resume(const char *resume_file, unsigned long long resume_offset)
{
	dev_t resume_device;
	int powerfd = -1;
	char device_string[64];
	ssize_t len;

	resume_device = name_to_dev_t(resume_file);

	if (major(resume_device) == 0) {
		fprintf(stderr, "Invalid resume device: %s\n", resume_file);
		goto failure;
	}

	if ((powerfd = open("/sys/power/resume", O_WRONLY)) < 0)
		goto fail_r;

	len = snprintf(device_string, sizeof device_string,
	               "%u:%u:%llu",
	               major(resume_device), minor(resume_device),
	               resume_offset);

	/* This should never happen */
	if (len >= (ssize_t) sizeof(device_string))
		goto fail_r;

	dprintf("kinit: trying to resume from %s\n", resume_file);

	if (write(powerfd, device_string, (size_t) len) != len)
		goto fail_r;

	/* Okay, what are we still doing alive... */
failure:
	if (powerfd >= 0)
		close(powerfd);
	dprintf("kinit: No resume image, doing normal boot...\n");
	return -1;

fail_r:
	fprintf(stderr, "Cannot write /sys/power/resume "
	        "(no software suspend kernel support?)\n");
	goto failure;
}
