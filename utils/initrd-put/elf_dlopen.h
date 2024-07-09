// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef __ELF_DLOPEN_H__
#define __ELF_DLOPEN_H__

#ifdef HAVE_LIBJSON_C
int process_json_metadata(const char *filename, const char *json, char *buf);
#else
inline int process_json_metadata(const char *, const char *, char *)
{
	return 0;
}
#endif

#endif /*__ELF_DLOPEN_H__ */
