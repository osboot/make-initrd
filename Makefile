.EXPORT_ALL_VARIABLES:

DIRS = \
	data \
	guess \
	features \
	tools \
	kmodule.deps.d

SUBDIRS = conf utils mk man datasrc

.PHONY: $(SUBDIRS)

all: $(SUBDIRS)

include Makefile.common

install: $(SUBDIRS)
	$(MKDIR_P) -- $(DESTDIR)$(tempdir)
	$(MKDIR_P) -- $(DESTDIR)$(datadir)/$(PROJECT)
	$(CP) -r -- $(DIRS) $(DESTDIR)$(datadir)/$(PROJECT)/

clean: $(SUBDIRS)

man: utils

$(SUBDIRS):
	$(Q)$(MAKE) $(MFLAGS) -C "$@" $(MAKECMDGOALS)

verify:
	find data features guess tools utils kmodule.deps.d -type f | \
	while read f; do \
	    ftype=$$(file -b "$$f"); \
	    [ -n "$${ftype##*shell script*}" ] || \
	    shellcheck -s dash -e SC1003,SC1090,SC1091,SC2004,SC2006,SC2015,SC2030,SC2031,SC2034,SC2086,SC2154 "$$f"; \
	done

TESTDIR ?= $(TOPDIR)/tests

check clean-check:
	$(Q)$(MAKE) $(MFLAGS) -C "$(TESTDIR)" $(MAKECMDGOALS)
