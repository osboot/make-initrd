TOPDIR = $(CURDIR)/..

MAN1PAGES = \
	make-initrd.1 \
	initrd-cp.1 \
	initrd-diff.1 \
	initrd-ls.1

MANPAGES = $(MAN1PAGES)
GARBAGE  = $(MANPAGES)

.PHONY: all install

all: $(MANPAGES)

include $(TOPDIR)/Makefile.common

%.1: $(TOPDIR)/utils/% %.1.inc
	$(HELP2MAN) -i $@.inc -- $< >$@

install: $(MANPAGES)
	$(MKDIR_P) -- $(DESTDIR)$(man1dir)
	$(CP) $(MAN1PAGES) $(DESTDIR)$(man1dir)/
