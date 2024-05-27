# SPDX-License-Identifier: GPL-3.0-or-later

make_initrd_DEST = $(dest_sbindir)/make-initrd
make_initrd_SRCS = $(utils_srcdir)/make-initrd/make-initrd.in

SCRIPTS += make_initrd
