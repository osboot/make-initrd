# SPDX-License-Identifier: GPL-3.0-or-later

gen_init_cpio_DEST = $(dest_bindir)/gen_init_cpio
gen_init_cpio_SRCS = utils/gen_init_cpio/gen_init_cpio.c
gen_init_cpio_LIBS =
gen_init_cpio_CFLAGS = -Wno-sign-conversion -Wno-discarded-qualifiers

PROGS += gen_init_cpio
