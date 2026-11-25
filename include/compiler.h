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
#ifndef _used_
# define _used_            __attribute__((__used__))
#endif
#ifndef _always_inline_
# define _always_inline_   inline __attribute__((__always_inline__))
#endif
#ifndef _noinline_
# define _noinline_        __attribute__((__noinline__))
#endif
#ifndef _packed_
# define _packed_          __attribute__((__packed__))
#endif
#ifndef _aligned_
# define _aligned_(x)      __attribute__((__aligned__(x)))
#endif
#ifndef _alignas_
# define _alignas_(x)      __attribute__((__aligned__(__alignof__(x))))
#endif
#ifndef _alignptr_
# define _alignptr_        __attribute__((__aligned__(sizeof(void *))))
#endif
#ifndef _fallthrough_
# define _fallthrough_     __attribute__((__fallthrough__))
#endif
#ifndef _alloc_
# define _alloc_(...)      __attribute__((__alloc_size__(__VA_ARGS__)))
#endif
#ifndef _access_
# if !defined(__clang__)
#  define _access_(...)             __attribute__((__access__(__VA_ARGS__)))
# else
#  define _access_(...)
# endif
# define _rdonly_ptr_(ptr_idx, ...) _nonnull_(ptr_idx) _access_(read_only, ptr_idx __VA_OPT__(,) __VA_ARGS__)
# define _wronly_ptr_(ptr_idx, ...) _nonnull_(ptr_idx) _access_(write_only, ptr_idx __VA_OPT__(,) __VA_ARGS__)
# define _rdwr_ptr_(ptr_idx, ...)   _nonnull_(ptr_idx) _access_(read_write, ptr_idx __VA_OPT__(,) __VA_ARGS__)
#endif
#ifndef _malloc_
# define _malloc_          __attribute__((__malloc__))
#endif
#ifndef _returns_nonnull_
# define _returns_nonnull_ __attribute__((__returns_nonnull__))
#endif
#ifndef _pure_
# define _pure_            __attribute__((__pure__))
#endif
#ifndef _const_
# define _const_           __attribute__((__const__))
#endif
#ifndef _weak_
# define _weak_            __attribute__((__weak__))
#endif
#ifndef _alias_
# define _alias_(x)        __attribute__((__alias__(#x)))
#endif
#ifndef _section_
# define _section_(x)      __attribute__((__section__(x)))
#endif
#ifndef _likely_
# define _likely_(x)       (__builtin_expect(!!(x), 1))
#endif
#ifndef _unlikely_
# define _unlikely_(x)     (__builtin_expect(!!(x), 0))
#endif

#ifndef ARRAY_SIZE
# define ARRAY_SIZE(x)                                                   \
	(__builtin_choose_expr(                                          \
		!__builtin_types_compatible_p(__typeof__(x), __typeof__(&*(x))), \
		sizeof(x) / sizeof((x)[0]),                              \
		(void)0))
#endif

#endif /* _COMPILER_H */
