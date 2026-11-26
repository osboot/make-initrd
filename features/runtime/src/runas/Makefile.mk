# SPDX-License-Identifier: GPL-3.0-or-later

runas_DEST = $(dest_data_bindir)/runas
runas_SRCS = $(runtime_srcdir)/runas/runas.c
runas_CFLAGS = $(runtime_CFLAGS)
runas_LDFLAGS = $(runtime_LDFLAGS)

PROGS += runas
