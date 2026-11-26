# SPDX-License-Identifier: GPL-3.0-or-later

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

initrd_runtime_SRCS += $(nfsmount_SRCS)
initrd_runtime_SYMLINKS += $(dest_data_bindir)/nfsmount

$(patsubst $(srcdir)/%.c,$(OBJDIR)/%.o,$(filter %.c,$(nfsmount_SRCS))): \
	extra_cflags += -Wno-conversion -Wno-sign-conversion -Wno-stringop-truncation
