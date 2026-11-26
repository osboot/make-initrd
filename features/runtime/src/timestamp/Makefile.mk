# SPDX-License-Identifier: GPL-3.0-or-later

timestamp_DEST = $(dest_data_sbindir)/timestamp
timestamp_SRCS = $(runtime_srcdir)/timestamp/timestamp.c
timestamp_CFLAGS = $(runtime_CFLAGS)
timestamp_LDFLAGS = $(runtime_LDFLAGS)

PROGS += timestamp
