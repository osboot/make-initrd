TOPDIR = $(CURDIR)/..

include $(TOPDIR)/Makefile.common

CONF = initrd.mk
CONF_EXAMPLES = $(wildcard initrd.mk.d/*.example)

.PHONY: all install clean

all:

install:
	$(MKDIR_P) -- $(DESTDIR)$(sysconfdir)/initrd.mk.d
	$(CP) $(CONF) $(DESTDIR)$(sysconfdir)/
	$(CP) $(CONF_EXAMPLES) $(DESTDIR)$(sysconfdir)/initrd.mk.d/

clean:
