// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef __RD_PATH_H__
#define __RD_PATH_H__

#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>

#include "compiler.h"

_public_ int rd_is_dot_dir(const struct dirent *ent)                           _rdonly_ptr_(1);
_public_ DIR *rd_opendir_or_die(const char *path)                              _rdonly_ptr_(1) _returns_nonnull_ _must_check_;
_public_ struct dirent *rd_readdir_or_die(DIR *d, const char *path)            _nonnull_(1) _rdonly_ptr_(2);
_public_ int rd_empty_directory(const char *path)                              _rdonly_ptr_(1);
_public_ ssize_t rd_read_retry(int fd, void *buf, size_t count)                _wronly_ptr_(2, 3);
_public_ ssize_t rd_write_retry(int fd, const void *buf, size_t count)         _rdonly_ptr_(2, 3);
_public_ ssize_t rd_write_loop(int fd, const char *buffer, size_t count)       _rdonly_ptr_(2, 3);

#endif /* __RD_PATH_H__ */
