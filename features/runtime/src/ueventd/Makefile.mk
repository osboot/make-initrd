# SPDX-License-Identifier: GPL-3.0-or-later

ueventd_DEST = $(dest_data_sbindir)/ueventd
ueventd_SRCS = \
	$(runtime_srcdir)/ueventd/path.c \
	$(runtime_srcdir)/ueventd/process.c \
	$(runtime_srcdir)/ueventd/queue-processor.c \
	$(runtime_srcdir)/ueventd/ueventd.c \
	$(runtime_srcdir)/ueventd/ueventd.h

ueventd_CFLAGS = -D_GNU_SOURCE -I$(runtime_srcdir)/libinitramfs
ueventd_LIBS = -L$(dest_data_libdir) -linitramfs

PROGS += ueventd
