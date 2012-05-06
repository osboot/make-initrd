CFLAGS = -Wall -Wextra -W -Wshadow -Wcast-align \
	-Wwrite-strings -Wconversion -Waggregate-return -Wstrict-prototypes \
	-Wmissing-prototypes -Wmissing-declarations -Wmissing-noreturn \
	-Wmissing-format-attribute -Wredundant-decls -Wdisabled-optimization \
	-Wno-pointer-arith

depinfo_LIBS   = -lkmod

PROGS = gen_init_cpio depinfo elfcheck

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

elfcheck: elfcheck.c
	$(CC) $(CFLAGS) -DPACKAGE=\"$@\" -DVERSION=\"1.0\" -o $@ $<

install: $(PROGS)
	$(MKDIR_P) -- $(DESTDIR)$(bindir)
	$(INSTALL) -p -m755 $(PROGS) $(DESTDIR)$(bindir)/

clean:
	rm -f -- $(PROGS)
