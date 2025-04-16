# SPDX-License-Identifier: GPL-3.0-or-later

halt_DEST = $(dest_data_bindir)/halt
halt_SRCS = \
	$(runtime_srcdir)/halt/halt.c \
	$(runtime_srcdir)/halt/hddown.c \
	$(runtime_srcdir)/halt/ifdown.c \
	$(runtime_srcdir)/halt/utmp.c \
	$(runtime_srcdir)/halt/reboot.h

PROGS += halt

halt_SYMLINKS = \
	$(dest_data_bindir)/reboot \
	$(dest_data_bindir)/poweroff
