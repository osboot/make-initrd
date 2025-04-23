# SPDX-License-Identifier: GPL-3.0-or-later

libinitramfs_DEST = $(dest_data_libdir)/libinitramfs.so
libinitramfs_SRCS = \
	$(runtime_srcdir)/libinitramfs/logging.c \
	$(runtime_srcdir)/libinitramfs/memory.c \
	$(runtime_srcdir)/libinitramfs/console.c \
	$(NULL)

libinitramfs_CFLAGS = \
	-fPIC \
	-I$(runtime_srcdir)/libinitramfs \
	-D_GNU_SOURCE

libinitramfs_LDFLAGS = -shared -fPIC
libinitramfs_LDFLAGS += -Wl,--enable-new-dtags
libinitramfs_LDFLAGS += -Wl,-z,combreloc
libinitramfs_LDFLAGS += -Wl,-soname="libinitramfs.so.$(PACKAGE_VERSION)"
libinitramfs_LDFLAGS += -Wl,--sort-common
libinitramfs_LDFLAGS += -Wl,--sort-section,alignment

libinitramfs_SYMLINKS = $(dest_data_libdir)/libinitramfs.so.$(PACKAGE_VERSION)

LIBS += libinitramfs
