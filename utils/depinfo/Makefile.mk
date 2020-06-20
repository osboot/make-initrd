ifeq ($(HAVE_LIBKMOD),yes)
debinfo_DEST = $(dest_sbindir)/depinfo
depinfo_SRCS = utils/depinfo/kmod-depinfo.c
depinfo_LIBS = $(shell pkg-config --libs libkmod)

PROGS     += $(debinfo_DEST)
C_SOURCES += $(depinfo_SRCS)
endif
