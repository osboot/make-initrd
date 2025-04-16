// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * reboot.h	Headerfile that defines how to handle
 *		the reboot() system call.
 *
 * Version:	@(#)reboot.h  2.85-17  04-Jun-2004  miquels@cistron.nl
 *
 * Copyright (C) (C) 1991-2004 Miquel van Smoorenburg.
 */

#include <sys/reboot.h>

#ifdef RB_ENABLE_CAD
#  define BMAGIC_HARD		RB_ENABLE_CAD
#endif

#ifdef RB_DISABLE_CAD
#  define BMAGIC_SOFT		RB_DISABLE_CAD
#endif

#ifdef RB_HALT_SYSTEM
#  define BMAGIC_HALT		RB_HALT_SYSTEM
#else
#  define BMAGIC_HALT		RB_HALT
#endif

#define BMAGIC_REBOOT		RB_AUTOBOOT

#ifdef RB_POWER_OFF
#  define BMAGIC_POWEROFF	RB_POWER_OFF
#elif defined(RB_POWEROFF)
#  define BMAGIC_POWEROFF	RB_POWEROFF
#else
#  define BMAGIC_POWEROFF	BMAGIC_HALT
#endif

/* for kexec support */
#ifdef RB_KEXEC
#  define BMAGIC_KEXEC		RB_KEXEC
#else
#  define BMAGIC_KEXEC		0x45584543
#endif

#define init_reboot(magic)	reboot(magic)

extern int ifdown(void);
extern int hddown(void);
extern int hdflush(void);
extern void write_wtmp(const char *user, const char *id, int pid, short type, const char *line);

