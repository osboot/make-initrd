# SPDX-License-Identifier: GPL-3.0-or-later

environ_DEST = $(dest_data_bindir)/environ
environ_SRCS = $(runtime_srcdir)/environ/environ.c

PROGS += environ

environ_CFLAGS = -D_GNU_SOURCE -I$(runtime_srcdir)/libinitramfs
environ_LIBS = -L$(dest_data_libdir) -linitramfs

environ_SYMLINKS += $(dest_data_bindir)/showenv
