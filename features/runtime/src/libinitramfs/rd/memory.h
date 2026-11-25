// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef __RD_MEMORY_H__
#define __RD_MEMORY_H__

#include <stdlib.h>

#include "compiler.h"

char *rd_strdup_or_die(const char *s)                      _nonnull_(1) _returns_nonnull_ _must_check_;
void *rd_calloc_or_die(size_t nmemb, size_t size)          _alloc_(1, 2) _returns_nonnull_ _must_check_;
void *rd_malloc_or_die(size_t size)                        _alloc_(1) _returns_nonnull_ _must_check_;
char *rd_asprintf_or_die(const char *fmt, ...)             _nonnull_(1) _printf_(1, 2) _returns_nonnull_
                                                           _must_check_;

#endif /* __RD_MEMORY_H__ */
