TOPDIR = $(CURDIR)/..

include $(TOPDIR)/Makefile.common

MAN1PAGES = \
	make-initrd.1 \
	initrd-diff.1

MANPAGES  = $(MAN1PAGES)

.PHONY: all install clean

%.1: $(TOPDIR)/utils/% %.1.inc
	$(HELP2MAN) -i $@.inc -- $< >$@

all: $(MANPAGES)

install: $(MANPAGES)
	$(MKDIR_P) -- $(DESTDIR)$(man1dir)
	$(CP) $(MAN1PAGES) $(DESTDIR)$(man1dir)/

clean:
	$(RM) -- $(MANPAGES)
