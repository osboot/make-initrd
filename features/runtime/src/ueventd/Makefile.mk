# SPDX-License-Identifier: GPL-3.0-or-later

initrd_runtime_SRCS += \
	$(runtime_srcdir)/ueventd/queue-processor.c \
	$(runtime_srcdir)/ueventd/sources.c \
	$(runtime_srcdir)/ueventd/ueventd.c \
	$(runtime_srcdir)/ueventd/ueventd.h

initrd_runtime_SYMLINKS += $(dest_data_sbindir)/ueventd
