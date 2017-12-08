TOPDIR = $(CURDIR)/..

include $(TOPDIR)/Makefile.common

TARGETS = \
	config.mk \
	guess.mk \
	rules.mk \
	initfiles.mk \
	make-initrd.mk

.PHONY:	all install clean

all: $(TARGETS)

install: $(TARGETS)
	$(MKDIR_P) -- $(DESTDIR)$(datadir)/$(PROJECT)/mk
	$(CP) -- $(TARGETS) $(DESTDIR)$(datadir)/$(PROJECT)/mk/

clean:
	$(Q)for n in *.in; do \
		rm -f -- "$${n%%.in}";\
	done
