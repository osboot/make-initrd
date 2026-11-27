// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MKRD_ELF_DLOPEN_H
#define MKRD_ELF_DLOPEN_H

#include "compiler.h"

#ifdef HAVE_LIBJSON_C
int process_json_metadata(const char *filename, const char *json, char *buf)
	_rdonly_ptr_(1) _rdonly_ptr_(2) _wronly_ptr_(3);
#else
static inline int process_json_metadata(const char *, const char *, char *)
{
	return 0;
}
#endif

#endif /*MKRD_ELF_DLOPEN_H */
