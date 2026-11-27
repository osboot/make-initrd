# SPDX-License-Identifier: GPL-3.0-or-later

initrd_runtime_SRCS += \
	$(runtime_srcdir)/halt/halt.c \
	$(runtime_srcdir)/halt/hddown.c \
	$(runtime_srcdir)/halt/ifdown.c \
	$(runtime_srcdir)/halt/utmp.c \
	$(runtime_srcdir)/halt/reboot.h

initrd_runtime_SYMLINKS += \
	$(dest_data_bindir)/halt \
	$(dest_data_bindir)/reboot \
	$(dest_data_bindir)/poweroff
