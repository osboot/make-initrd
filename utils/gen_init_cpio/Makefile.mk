gen_init_cpio_DEST = $(dest_bindir)/gen_init_cpio
gen_init_cpio_SRCS = utils/gen_init_cpio/gen_init_cpio.c
gen_init_cpio_LIBS =

PROGS     += $(gen_init_cpio_DEST)
C_SOURCES += $(gen_init_cpio_SRCS)
