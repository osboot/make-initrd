// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef _CLEANUP_H
#define _CLEANUP_H 1

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include "compiler.h"

#define __cleanup(func)   __attribute__((__cleanup__(func)))

#define DEFINE_FREE(name, type, free_expr) \
	static inline void __free_##name(void *p) { type _t = *(type *)p; free_expr; }

#define __free(name)      __cleanup(__free_##name)

#define no_free_ptr(p) \
	({ __typeof__(p) __val = (p); (p) = NULL; __val; })

#define no_free_fd(fd) \
	({ __typeof__(fd) __fd = (fd); (fd) = -1; __fd; })

/* Common cleanup handlers */
DEFINE_FREE(free, void *, if (_t) free(_t))
DEFINE_FREE(close, int, if (_t >= 0) close(_t))
DEFINE_FREE(fclose, FILE *, if (_t) fclose(_t))

#endif /* _CLEANUP_H */
