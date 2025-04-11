// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 1991-2025 Free Software Foundation, Inc.

#ifndef _TEMP_FAILURE_RETRY_H
# define _TEMP_FAILURE_RETRY_H	1

# include <unistd.h>

# ifndef TEMP_FAILURE_RETRY
/* This implementation of TEMP_FAILURE_RETRY is taken from the glibc code. */
#  define TEMP_FAILURE_RETRY(expression)				\
   (__extension__							\
     ({ long int __result;						\
        do __result = (long int) (expression);				\
        while (__result == -1L && errno == EINTR);			\
        __result; }))

# endif
#endif
