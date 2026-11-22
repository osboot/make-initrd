# SPDX-License-Identifier: GPL-3.0-or-later

ueventd_net_DEST = $(dest_featuredir)/network/lib/uevent/sources/ueventd-net
ueventd_net_SRCDIR = $(features_srcdir)/network/src/ueventd-net

ueventd_net_SRCS = \
	$(ueventd_net_SRCDIR)/ueventd-net.c \
	$(ueventd_net_SRCDIR)/ueventd-net.h \
	$(ueventd_net_SRCDIR)/netlink.c \
	$(ueventd_net_SRCDIR)/state.c

ueventd_net_CFLAGS = -D_GNU_SOURCE -I$(features_srcdir)/runtime/src/libinitramfs
ueventd_net_LIBS = -L$(dest_data_libdir) -linitramfs

PROGS += ueventd_net
