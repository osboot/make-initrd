.EXPORT_ALL_VARIABLES:

include Makefile.common

DIRS = \
	data \
	guess \
	features \
	tools \
	kmodule.deps.d

SUBDIRS = conf utils mk man datasrc

.PHONY: $(SUBDIRS)

all: $(SUBDIRS)

install: $(SUBDIRS)
	$(MKDIR_P) -- $(DESTDIR)$(tempdir)
	$(MKDIR_P) -- $(DESTDIR)$(datadir)/$(PROJECT)
	$(CP) -r -- $(DIRS) $(DESTDIR)$(datadir)/$(PROJECT)/

clean: $(SUBDIRS)

man: utils

$(SUBDIRS):
	$(MAKE) $(MFLAGS) -C "$@" $(MAKECMDGOALS)
