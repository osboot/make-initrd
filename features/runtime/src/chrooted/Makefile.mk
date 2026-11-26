# SPDX-License-Identifier: GPL-3.0-or-later

chrooted_DEST = $(dest_data_bindir)/chrooted
chrooted_SRCS = $(runtime_srcdir)/chrooted/chrooted.c
chrooted_CFLAGS = $(runtime_CFLAGS)
chrooted_LDFLAGS = $(runtime_LDFLAGS)

PROGS += chrooted
