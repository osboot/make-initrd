# SPDX-License-Identifier: GPL-3.0-or-later

ueventd_mounts_SRCDIR = $(runtime_srcdir)/ueventd-mounts

initrd_runtime_SRCS += \
	$(ueventd_mounts_SRCDIR)/ueventd-mounts.c \
	$(ueventd_mounts_SRCDIR)/ueventd-mounts.h \
	$(ueventd_mounts_SRCDIR)/mount-compat.h \
	$(ueventd_mounts_SRCDIR)/backend-fanotify.c \
	$(ueventd_mounts_SRCDIR)/backend-procfs.c

initrd_runtime_SYMLINKS += $(dest_data_ueventdir)/ueventd-mounts
