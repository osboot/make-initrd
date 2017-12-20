TOPDIR = $(CURDIR)/..

include $(TOPDIR)/Makefile.common

CFLAGS = $(warning_CFLAGS) \
	-DPACKAGE=\"$@\" -DVERSION=\"$(VERSION)\"

CFLAGS += -DHAVE_GZIP -DHAVE_BZIP2 -DHAVE_LZMA

gen_init_cpio_SRCS = gen_init_cpio.c
gen_init_cpio_LIBS =

depinfo_SRCS = kmod-depinfo.c
depinfo_LIBS = $(shell pkg-config --libs libkmod)

initrd_ls_SRCS = initrd-ls.c initrd-ls-format.c initrd-common.c initrd-cpio.c initrd-parse.c initrd-decompress.c
initrd_ls_LIBS =

initrd_ls_SRCS += initrd-decompress-gzip.c
initrd_ls_LIBS += $(shell pkg-config --libs zlib)

initrd_ls_SRCS += initrd-decompress-bzip2.c
initrd_ls_LIBS += -lbz2

initrd_ls_SRCS += initrd-decompress-lzma.c
initrd_ls_LIBS += $(shell pkg-config --libs liblzma)

initrd_extract_SRCS = initrd-extract.c initrd-common.c initrd-cpio.c initrd-parse.c initrd-decompress.c
initrd_extract_LIBS =

initrd_extract_SRCS += initrd-decompress-gzip.c
initrd_extract_LIBS += $(shell pkg-config --libs zlib)

initrd_extract_SRCS += initrd-decompress-bzip2.c
initrd_extract_LIBS += -lbz2

initrd_extract_SRCS += initrd-decompress-lzma.c
initrd_extract_LIBS += $(shell pkg-config --libs liblzma)

bin_PROGS  = \
	gen_init_cpio

sbin_PROGS = \
	depinfo \
	initrd-cp \
	initrd-ls \
	initrd-extract \
	initrd-diff \
	make-initrd \
	mkinitrd-make-initrd

DEPS = $(call get_depends,$(bin_PROGS) $(sbin_PROGS),)
OBJS = $(call get_objects,$(bin_PROGS) $(sbin_PROGS),)

.PHONY: all install clean

all: $(bin_PROGS) $(sbin_PROGS)

gen_init_cpio: $(call get_objects,gen_init_cpio)
	$(LINK) $^ $(gen_init_cpio_LIBS) -o $@

depinfo: $(call get_objects,depinfo)
	$(LINK) $^ $(depinfo_LIBS) -o $@

initrd-ls: $(call get_objects,initrd-ls)
	$(LINK) $^ $(initrd_ls_LIBS) -o $@

initrd-extract: $(call get_objects,initrd-extract)
	$(LINK) $^ $(initrd_extract_LIBS) -o $@

install: $(bin_PROGS) $(sbin_PROGS)
	$(MKDIR_P) -- $(DESTDIR)$(bindir)
	$(INSTALL) -p -m755 $(bin_PROGS) $(DESTDIR)$(bindir)/
	$(MKDIR_P) -- $(DESTDIR)$(sbindir)
	$(INSTALL) -p -m755 $(sbin_PROGS) $(DESTDIR)$(sbindir)/

clean:
	$(RM) -- $(bin_PROGS) $(sbin_PROGS) $(DEPS) $(OBJS)

# We need dependencies only if goal isn't "indent" or "clean".
ifneq ($(MAKECMDGOALS),indent)
ifneq ($(MAKECMDGOALS),clean)

%.d: %.c Makefile
	$(DEP) -MM $(CPPFLAGS) $< |sed -e 's,\($*\)\.o[ :]*,\1.o $@: Makefile ,g' >$@

ifneq ($(DEPS),)
-include $(DEPS)
endif

endif # clean
endif # indent
