// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef _COMPILER_H
#define _COMPILER_H 1

#ifdef __has_include
# if __has_include(<features.h>)
#  include <features.h>
# endif
#endif

#ifndef __noreturn
# define __noreturn        __attribute__((__noreturn__))
#endif
#ifndef __printf
# define __printf(a, b)    __attribute__((__format__(printf, a, b)))
#endif
#undef __nonnull
#define __nonnull(...)     __attribute__((__nonnull__(__VA_ARGS__)))
#ifndef __must_check
# define __must_check      __attribute__((__warn_unused_result__))
#endif
#ifndef __maybe_unused
# define __maybe_unused    __attribute__((__unused__))
#endif
#ifndef __always_inline
# define __always_inline   inline __attribute__((__always_inline__))
#endif
#ifndef __packed
# define __packed          __attribute__((__packed__))
#endif
#ifndef __aligned
# define __aligned(x)      __attribute__((__aligned__(x)))
#endif
#ifndef __fallthrough
# define __fallthrough     __attribute__((__fallthrough__))
#endif

#endif /* _COMPILER_H */
