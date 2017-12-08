TOPDIR = $(CURDIR)/..

include $(TOPDIR)/Makefile.common

CFLAGS = -Wall -Wextra -W -Wshadow -Wcast-align \
	-Wwrite-strings -Wconversion -Waggregate-return -Wstrict-prototypes \
	-Wmissing-prototypes -Wmissing-declarations -Wmissing-noreturn \
	-Wmissing-format-attribute -Wredundant-decls -Wdisabled-optimization \
	-Wno-pointer-arith

CFLAGS += -DPACKAGE=\"$@\" -DVERSION=\"$(VERSION)\"

gen_init_cpio_SRC = gen_init_cpio.c
gen_init_cpio_LIB =
gen_init_cpio_OBJ = $(gen_init_cpio_SRC:.c=.o)
gen_init_cpio_DEP = $(gen_init_cpio_SRC:.c=.d)

depinfo_SRC = kmod-depinfo.c
depinfo_LIB = $(shell pkg-config --libs libkmod)
depinfo_OBJ = $(depinfo_SRC:.c=.o)
depinfo_DEP = $(depinfo_SRC:.c=.d)

initrd_ls_SRC = initrd-ls.c initrd-ls-format.c initrd-common.c initrd-cpio.c initrd-parse.c initrd-decompress.c
initrd_ls_LIB =
initrd_ls_OBJ = $(initrd_ls_SRC:.c=.o)
initrd_ls_DEP = $(initrd_ls_SRC:.c=.d)

initrd_ls_SRC += initrd-decompress-gzip.c
initrd_ls_LIB += $(shell pkg-config --libs zlib)

initrd_ls_SRC += initrd-decompress-bzip2.c
initrd_ls_LIB += -lbz2

initrd_ls_SRC += initrd-decompress-lzma.c
initrd_ls_LIB += $(shell pkg-config --libs liblzma)

initrd_extract_SRC = initrd-extract.c initrd-common.c initrd-cpio.c initrd-parse.c initrd-decompress.c
initrd_extract_LIB =
initrd_extract_OBJ = $(initrd_extract_SRC:.c=.o)
initrd_extract_DEP = $(initrd_extract_SRC:.c=.d)

initrd_extract_SRC += initrd-decompress-gzip.c
initrd_extract_LIB += $(shell pkg-config --libs zlib)

initrd_extract_SRC += initrd-decompress-bzip2.c
initrd_extract_LIB += -lbz2

initrd_extract_SRC += initrd-decompress-lzma.c
initrd_extract_LIB += $(shell pkg-config --libs liblzma)

DEP = $(gen_init_cpio_DEP) $(depinfo_DEP) $(initrd_ls_DEP) $(initrd_extract_DEP)

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

.PHONY:	all install clean

all: $(bin_PROGS) $(sbin_PROGS)

gen_init_cpio: $(gen_init_cpio_OBJ)
	$(LINK.o) $^ $(gen_init_cpio_LIB) -o $@

depinfo: $(depinfo_OBJ)
	$(LINK.o) $^ $(depinfo_LIB) -o $@

initrd-ls: $(initrd_ls_OBJ)
	$(LINK.o) $^ $(initrd_ls_LIB) -o $@

initrd-extract: $(initrd_extract_OBJ)
	$(LINK.o) $^ $(initrd_extract_LIB) -o $@

install: $(bin_PROGS) $(sbin_PROGS)
	$(MKDIR_P) -- $(DESTDIR)$(bindir)
	$(INSTALL) -p -m755 $(bin_PROGS) $(DESTDIR)$(bindir)/
	$(MKDIR_P) -- $(DESTDIR)$(sbindir)
	$(INSTALL) -p -m755 $(sbin_PROGS) $(DESTDIR)$(sbindir)/

clean:
	$(RM) -- \
	    $(bin_PROGS) $(sbin_PROGS) $(DEP) \
	    $(gen_init_cpio_OBJ) $(depinfo_OBJ) $(initrd_ls_OBJ) $(initrd_extract_OBJ)

# We need dependencies only if goal isn't "indent" or "clean".
ifneq ($(MAKECMDGOALS),indent)
ifneq ($(MAKECMDGOALS),clean)

%.d: %.c Makefile
	@echo Making dependences for $<
	@$(CC) -MM $(CPPFLAGS) $< |sed -e 's,\($*\)\.o[ :]*,\1.o $@: Makefile ,g' >$@

ifneq ($(DEP),)
-include $(DEP)
endif

endif # clean
endif # indent
