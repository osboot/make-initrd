# SPDX-License-Identifier: GPL-3.0-or-later

ueventd_mounts_DEST = $(dest_data_sbindir)/ueventd-mounts
ueventd_mounts_SRCDIR = datasrc/ueventd-mounts

ueventd_mounts_SRCS = $(ueventd_mounts_SRCDIR)/ueventd-mounts.c

ueventd_mounts_CFLAGS = -D_GNU_SOURCE -Idatasrc/libinitramfs
ueventd_mounts_LIBS = -L$(dest_data_libdir) -linitramfs

PROGS += ueventd_mounts
