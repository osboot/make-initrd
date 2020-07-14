ifeq ($(HAVE_LIBKMOD),yes)
depinfo_DEST = $(dest_sbindir)/depinfo
depinfo_SRCS = utils/depinfo/kmod-depinfo.c
depinfo_LIBS = $(HAVE_LIBKMOD_LIBS)
depinfo_CFLAGS = $(HAVE_LIBKMOD_CFLAGS)

PROGS += depinfo
endif
