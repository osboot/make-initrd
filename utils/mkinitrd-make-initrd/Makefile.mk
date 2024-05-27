# SPDX-License-Identifier: GPL-3.0-or-later

mkinitrd_make_initrd_DEST = $(dest_sbindir)/mkinitrd-make-initrd
mkinitrd_make_initrd_SRCS = $(utils_srcdir)/mkinitrd-make-initrd/mkinitrd-make-initrd.in

SCRIPTS += mkinitrd_make_initrd
