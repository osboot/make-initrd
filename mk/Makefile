TOPDIR = $(CURDIR)/..

GEN = $(patsubst %.in,%,$(wildcard *.in))

TARGETS = \
	config.mk \
	guess.mk \
	rules.mk \
	initfiles.mk \
	make-initrd.mk

GARBAGE = $(GEN)

.PHONY: all install

all: $(TARGETS)

include $(TOPDIR)/Makefile.common

install: $(TARGETS)
	$(MKDIR_P) -- $(DESTDIR)$(datadir)/$(PROJECT)/mk
	$(CP) -- $(TARGETS) $(DESTDIR)$(datadir)/$(PROJECT)/mk/
