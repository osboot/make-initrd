# SPDX-License-Identifier: GPL-3.0-or-later

make_initrd_shutdown_restore_DEST = $(dest_sbindir)/make-initrd-shutdown-restore
make_initrd_shutdown_restore_SRCS = $(utils_srcdir)/make-initrd-shutdown-restore/make-initrd-shutdown-restore.in

SCRIPTS += make_initrd_shutdown_restore
