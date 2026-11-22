# SPDX-License-Identifier: GPL-3.0-or-later

cpio_stream_DEST = $(dest_bindir)/cpio-stream
cpio_stream_SRCS = \
	$(utils_srcdir)/cpio-stream/cpio-stream.c \
	$(utils_srcdir)/initrd-cpio.c \
	$(utils_srcdir)/initrd-common.c \
	$(NULL)

cpio_stream_LIBS =
cpio_stream_CFLAGS += -I$(utils_srcdir)

PROGS += cpio_stream
