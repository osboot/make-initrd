/* SPDX-License-Identifier: GPL-2.0-or-later */

#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>

#include "ueventd.h"

int is_dot_dir(struct dirent *ent)
{
	return (ent->d_type == DT_DIR &&
	        (ent->d_name[0] == '.' && (ent->d_name[1] == '\0' ||
	                                   (ent->d_name[1] == '.' && ent->d_name[2] == '\0') )));
}

DIR *xopendir(const char *path)
{
	DIR *dir = opendir(path);
	if (!dir)
		fatal("opendir: %s: %m", path);
	return dir;
}


struct dirent *xreaddir(DIR *d, const char *path)
{
	struct dirent *ent;

	errno = 0;
	ent = readdir(d);
	if (!ent) {
		if (!errno)
			return NULL;
		fatal("readdir: %s: %m", path);
	}
	return ent;
}

int empty_directory(const char *path)
{
	struct dirent *ent;
	int is_empty = 1;
	DIR *d = xopendir(path);

	while ((ent = xreaddir(d, path)) != NULL) {
		if (ent->d_name[0] != '.') {
			is_empty = 0;
			break;
		}
	}
	closedir(d);

	return is_empty;
}

ssize_t read_retry(int fd, void *buf, size_t count)
{
	return TEMP_FAILURE_RETRY(read(fd, buf, count));
}

ssize_t write_retry(int fd, const void *buf, size_t count)
{
	return TEMP_FAILURE_RETRY(write(fd, buf, count));
}

ssize_t write_loop(int fd, const char *buffer, size_t count)
{
	ssize_t offset = 0;

	while (count > 0) {
		ssize_t block = write_retry(fd, &buffer[offset], count);

		if (block <= 0)
			return offset ? : block;
		offset += block;
		count -= (size_t) block;
	}
	return offset;
}
