# SPDX-License-Identifier: GPL-3.0-or-later

nfsmount_DEST = $(dest_data_bindir)/nfsmount
nfsmount_SRCS = \
	$(runtime_srcdir)/nfsmount/dummypmap.c \
	$(runtime_srcdir)/nfsmount/dummypmap.h \
	$(runtime_srcdir)/nfsmount/main.c \
	$(runtime_srcdir)/nfsmount/mount.c \
	$(runtime_srcdir)/nfsmount/nfsmount.h \
	$(runtime_srcdir)/nfsmount/portmap.c \
	$(runtime_srcdir)/nfsmount/sunrpc.c \
	$(runtime_srcdir)/nfsmount/sunrpc.h \
	$(NULL)

nfsmount_CFLAGS = -I$(runtime_srcdir) -D_GNU_SOURCE=1 -Wno-conversion -Wno-sign-conversion -Wno-stringop-truncation

PROGS += nfsmount
