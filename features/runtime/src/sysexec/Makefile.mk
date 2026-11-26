# SPDX-License-Identifier: GPL-3.0-or-later

sysexec_DEST = $(dest_data_sbindir)/sysexec
sysexec_SRCS = $(runtime_srcdir)/sysexec/sysexec.c

sysexec_CFLAGS = -D_GNU_SOURCE -I$(runtime_srcdir)/libinitramfs
sysexec_LIBS = -L$(dest_data_libdir) -linitramfs

PROGS += sysexec
