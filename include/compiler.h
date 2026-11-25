// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef _COMPILER_H
#define _COMPILER_H 1

#ifndef _noreturn_
# define _noreturn_        __attribute__((__noreturn__))
#endif
#ifndef _printf_
# define _printf_(a, b)    __attribute__((__format__(printf, a, b)))
#endif
#ifndef _nonnull_
# define _nonnull_(...)    __attribute__((__nonnull__(__VA_ARGS__)))
#endif
#ifndef _must_check_
# define _must_check_      __attribute__((__warn_unused_result__))
#endif
#ifndef _maybe_unused_
# define _maybe_unused_    __attribute__((__unused__))
#endif
#ifndef _always_inline_
# define _always_inline_   inline __attribute__((__always_inline__))
#endif
#ifndef _packed_
# define _packed_          __attribute__((__packed__))
#endif
#ifndef _aligned_
# define _aligned_(x)      __attribute__((__aligned__(x)))
#endif
#ifndef _fallthrough_
# define _fallthrough_     __attribute__((__fallthrough__))
#endif

#endif /* _COMPILER_H */
