ifeq ($(HAVE_LIBKMOD),yes)
initrd_scanmod_DEST = $(dest_sbindir)/initrd-scanmod
initrd_scanmod_SRCS = \
	utils/initrd-scanmod/initrd-scanmod.h \
	utils/initrd-scanmod/initrd-scanmod.c \
	utils/initrd-scanmod/initrd-scanmod-common.c \
	utils/initrd-scanmod/initrd-scanmod-file.c \
	utils/initrd-scanmod/initrd-scanmod-rules.c \
	utils/initrd-scanmod/initrd-scanmod-walk.c \
	$(NULL)

initrd_scanmod_LIBS = $(shell pkg-config --libs libkmod)

PROGS     += $(initrd_scanmod_DEST)
C_SOURCES += $(initrd_scanmod_SRCS)
endif
