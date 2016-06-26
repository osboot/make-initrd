TOPDIR = $(CURDIR)/..

include $(TOPDIR)/Makefile.common

CFLAGS = -Wall -Wextra -W -Wshadow -Wcast-align \
	-Wwrite-strings -Wconversion -Waggregate-return -Wstrict-prototypes \
	-Wmissing-prototypes -Wmissing-declarations -Wmissing-noreturn \
	-Wmissing-format-attribute -Wredundant-decls -Wdisabled-optimization \
	-Wno-pointer-arith

depinfo_LIBS = $(shell pkg-config --libs libkmod)

initrd_ls_CFLAGS =
initrd_ls_LIBS   =

initrd_ls_HEADERS = \
	initrd-common.h \
	initrd-cpio.h \
	initrd-decompress.h \
	initrd-parse.h \
	initrd-ls.h

initrd_ls_SOURCES = \
	initrd-ls.c \
	initrd-ls-format.c \
	initrd-common.c \
	initrd-cpio.c \
	initrd-parse.c \
	initrd-decompress.c

initrd_ls_CFLAGS  += -DHAVE_GZIP
initrd_ls_SOURCES += initrd-decompress-gzip.c
initrd_ls_LIBS    += $(shell pkg-config --libs zlib)

initrd_ls_CFLAGS  += -DHAVE_BZIP2
initrd_ls_SOURCES += initrd-decompress-bzip2.c
initrd_ls_LIBS    += -lbz2

initrd_ls_CFLAGS  += -DHAVE_LZMA
initrd_ls_SOURCES += initrd-decompress-lzma.c
initrd_ls_LIBS    += $(shell pkg-config --libs liblzma)

bin_PROGS  = \
	gen_init_cpio
sbin_PROGS = \
	depinfo \
	initrd-ls \
	initrd-diff \
	make-initrd \
	mkinitrd-make-initrd

.PHONY:	all install clean

all: $(bin_PROGS) $(sbin_PROGS)

gen_init_cpio: gen_init_cpio.c
	$(CC) $(CFLAGS) -o $@ $<

depinfo: kmod-depinfo.c
	$(CC) $(CFLAGS) -DPACKAGE=\"$@\" -DVERSION=\"1.0\" -o $@ $< $(depinfo_LIBS)

initrd-ls: $(initrd_ls_HEADERS) $(initrd_ls_SOURCES)
	$(CC) $(CFLAGS) $(initrd_ls_CFLAGS) -DPACKAGE=\"$@\" -DVERSION=\"1.0\" -o $@ $(initrd_ls_SOURCES) $(initrd_ls_LIBS)

install: $(bin_PROGS) $(sbin_PROGS)
	$(MKDIR_P) -- $(DESTDIR)$(bindir)
	$(INSTALL) -p -m755 $(bin_PROGS) $(DESTDIR)$(bindir)/
	$(MKDIR_P) -- $(DESTDIR)$(sbindir)
	$(INSTALL) -p -m755 $(sbin_PROGS) $(DESTDIR)$(sbindir)/

clean:
	$(RM) -- $(bin_PROGS) $(sbin_PROGS)
