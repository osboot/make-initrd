# SPDX-License-Identifier: GPL-3.0-or-later

ifeq ($(HAVE_LIBKMOD),yes)
initrd_scanmod_DEST = $(dest_sbindir)/initrd-scanmod
initrd_scanmod_SRCS = \
	$(utils_srcdir)/initrd-scanmod/initrd-scanmod.h \
	$(utils_srcdir)/initrd-scanmod/initrd-scanmod.c \
	$(utils_srcdir)/initrd-scanmod/initrd-scanmod-common.c \
	$(utils_srcdir)/initrd-scanmod/initrd-scanmod-file.c \
	$(utils_srcdir)/initrd-scanmod/initrd-scanmod-rules.c \
	$(utils_srcdir)/initrd-scanmod/initrd-scanmod-walk.c \
	$(NULL)

initrd_scanmod_LIBS = $(HAVE_LIBKMOD_LIBS)

PROGS += initrd_scanmod
endif
