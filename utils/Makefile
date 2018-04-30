TOPDIR = $(CURDIR)/..

CFLAGS = $(warning_CFLAGS) \
	-DPACKAGE=\"$@\" -DVERSION=\"$(VERSION)\"

CFLAGS += -DHAVE_GZIP -DHAVE_BZIP2 -DHAVE_LZMA -DHAVE_ZSTD

gen_init_cpio_SRCS = gen_init_cpio.c
gen_init_cpio_LIBS =

depinfo_SRCS = kmod-depinfo.c
depinfo_LIBS = $(shell pkg-config --libs libkmod)

initrd_scanmod_SRCS = initrd-scanmod.c initrd-scanmod-common.c initrd-scanmod-file.c initrd-scanmod-rules.c initrd-scanmod-walk.c
initrd_scanmod_LIBS = $(shell pkg-config --libs libkmod)

initrd_ls_SRCS = initrd-ls.c initrd-ls-format.c initrd-common.c initrd-cpio.c initrd-parse.c initrd-decompress.c
initrd_ls_LIBS =

initrd_ls_SRCS += initrd-decompress-gzip.c
initrd_ls_LIBS += $(shell pkg-config --libs zlib)

initrd_ls_SRCS += initrd-decompress-bzip2.c
initrd_ls_LIBS += -lbz2

initrd_ls_SRCS += initrd-decompress-lzma.c
initrd_ls_LIBS += $(shell pkg-config --libs liblzma)

initrd_ls_SRCS += initrd-decompress-zstd.c
initrd_ls_LIBS += $(shell pkg-config --libs libzstd)

initrd_extract_SRCS = initrd-extract.c initrd-common.c initrd-cpio.c initrd-parse.c initrd-decompress.c
initrd_extract_LIBS =

initrd_extract_SRCS += initrd-decompress-gzip.c
initrd_extract_LIBS += $(shell pkg-config --libs zlib)

initrd_extract_SRCS += initrd-decompress-bzip2.c
initrd_extract_LIBS += -lbz2

initrd_extract_SRCS += initrd-decompress-lzma.c
initrd_extract_LIBS += $(shell pkg-config --libs liblzma)

initrd_extract_SRCS += initrd-decompress-zstd.c
initrd_extract_LIBS += $(shell pkg-config --libs libzstd)

bin_PROGS  = \
	gen_init_cpio

sbin_PROGS = \
	depinfo \
	initrd-scanmod \
	initrd-cp \
	initrd-ls \
	initrd-extract \
	initrd-diff \
	make-initrd \
	mkinitrd-make-initrd

DEPS = $(call get_depends,$(bin_PROGS) $(sbin_PROGS),)
OBJS = $(call get_objects,$(bin_PROGS) $(sbin_PROGS),)

GARBAGE   = $(bin_PROGS) $(sbin_PROGS) $(DEPS) $(OBJS)
C_SOURCES = *.h $(depinfo_SRCS) $(initrd_ls_SRCS) $(initrd_extract_SRCS) $(initrd_scanmod_SRCS)

all: $(bin_PROGS) $(sbin_PROGS)

include $(TOPDIR)/Makefile.common

gen_init_cpio: $(call get_objects,gen_init_cpio)
	$(LINK) $^ $(gen_init_cpio_LIBS) -o $@

depinfo: $(call get_objects,depinfo)
	$(LINK) $^ $(depinfo_LIBS) -o $@

initrd-scanmod: $(call get_objects,initrd-scanmod)
	$(LINK) $^ $(initrd_scanmod_LIBS) -o $@

initrd-ls: $(call get_objects,initrd-ls)
	$(LINK) $^ $(initrd_ls_LIBS) -o $@

initrd-extract: $(call get_objects,initrd-extract)
	$(LINK) $^ $(initrd_extract_LIBS) -o $@

install: $(bin_PROGS) $(sbin_PROGS)
	$(MKDIR_P) -- $(DESTDIR)$(bindir)
	$(INSTALL) -p -m755 $(bin_PROGS) $(DESTDIR)$(bindir)/
	$(MKDIR_P) -- $(DESTDIR)$(sbindir)
	$(INSTALL) -p -m755 $(sbin_PROGS) $(DESTDIR)$(sbindir)/
