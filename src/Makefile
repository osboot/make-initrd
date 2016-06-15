CFLAGS = -Wall -Wextra -W -Wshadow -Wcast-align \
	-Wwrite-strings -Wconversion -Waggregate-return -Wstrict-prototypes \
	-Wmissing-prototypes -Wmissing-declarations -Wmissing-noreturn \
	-Wmissing-format-attribute -Wredundant-decls -Wdisabled-optimization \
	-Wno-pointer-arith

depinfo_LIBS   = -lkmod

initrd_ls_LIBS = -lz -lbz2

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
	initrd-decompress.c \
	initrd-decompress-gzip.c \
	initrd-decompress-bzip2.c

PROGS = gen_init_cpio depinfo initrd-ls

bindir  ?= /usr/bin
DESTDIR ?=

INSTALL ?= install
MKDIR_P ?= mkdir -p

.PHONY:	all install clean

all: $(PROGS)

gen_init_cpio: gen_init_cpio.c
	$(CC) $(CFLAGS) -o $@ $<

depinfo: kmod-depinfo.c
	$(CC) $(CFLAGS) -DPACKAGE=\"$@\" -DVERSION=\"1.0\" -o $@ $< $(depinfo_LIBS)

initrd-ls: $(initrd_ls_HEADERS) $(initrd_ls_SOURCES)
	$(CC) $(CFLAGS) -DPACKAGE=\"$@\" -DVERSION=\"1.0\" -o $@ $(initrd_ls_SOURCES) $(initrd_ls_LIBS)

install: $(PROGS)
	$(MKDIR_P) -- $(DESTDIR)$(bindir)
	$(INSTALL) -p -m755 $(PROGS) $(DESTDIR)$(bindir)/

clean:
	rm -f -- $(PROGS)
