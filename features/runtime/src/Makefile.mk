# SPDX-License-Identifier: GPL-3.0-or-later

runtime_srcdir = $(features_srcdir)/runtime/src

initrd_runtime_DEST = $(dest_data_bindir)/runtime
initrd_runtime_SRCS = $(runtime_srcdir)/runtime.c $(runtime_srcdir)/runtime.h
initrd_runtime_CFLAGS = $(runtime_CFLAGS) -D_GNU_SOURCE -I$(runtime_srcdir) -I$(runtime_srcdir)/libinitramfs
initrd_runtime_LDFLAGS = $(runtime_LDFLAGS)
initrd_runtime_LIBS = -L$(dest_data_libdir) -linitramfs
ifneq ($(MKLOCAL),no)
initrd_runtime_LIBS += -Wl,-rpath,$(abspath $(dest_data_libdir))
endif
initrd_runtime_SYMLINKS =

include $(runtime_srcdir)/*/Makefile.mk

PROGS += initrd_runtime
