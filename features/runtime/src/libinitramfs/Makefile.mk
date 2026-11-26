# SPDX-License-Identifier: GPL-3.0-or-later

libinitramfs_DEST = $(dest_data_libdir)/libinitramfs.so
libinitramfs_SRCS = \
	$(runtime_srcdir)/libinitramfs/logging.c \
	$(runtime_srcdir)/libinitramfs/memory.c \
	$(runtime_srcdir)/libinitramfs/path.c \
	$(runtime_srcdir)/libinitramfs/process.c \
	$(runtime_srcdir)/libinitramfs/uevent.c \
	$(NULL)

libinitramfs_CFLAGS = \
	$(runtime_CFLAGS) \
	-fPIC \
	-I$(runtime_srcdir)/libinitramfs \
	-D_GNU_SOURCE

libinitramfs_LDFLAGS = \
	$(runtime_LDFLAGS) \
	-shared -fPIC \
	-Wl,-soname="libinitramfs.so.$(PACKAGE_VERSION)"

libinitramfs_SYMLINKS = $(dest_data_libdir)/libinitramfs.so.$(PACKAGE_VERSION)

LIBS += libinitramfs
