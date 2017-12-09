TOPDIR = $(CURDIR)/..

include $(TOPDIR)/Makefile.common

GEN = $(patsubst %.in,%,$(wildcard *.in))

TARGETS = \
	config.mk \
	guess.mk \
	rules.mk \
	initfiles.mk \
	make-initrd.mk

.PHONY: all install clean

all: $(TARGETS)

install: $(TARGETS)
	$(MKDIR_P) -- $(DESTDIR)$(datadir)/$(PROJECT)/mk
	$(CP) -- $(TARGETS) $(DESTDIR)$(datadir)/$(PROJECT)/mk/

clean:
	$(RM) -- $(GEN)
