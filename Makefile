PROJECT = make-initrd
VERSION = 0.1.0

sysconfdir = /etc
bindir     = /usr/bin
datadir    = /usr/share
workdir    = /tmp
DESTDIR   ?=

CP = cp -a
INSTALL = install
MKDIR_P = mkdir -p
TOUCH_R = touch -r

DIRS = autodetect data features tools

CONF = initrd.mk

bin_TARGETS = make-initrd

TARGETS = config.mk rules.mk build.mk

SUBDIRS = src

.PHONY:	$(SUBDIRS)

all: $(SUBDIRS) $(TARGETS) $(bin_TARGETS)

%: %.in
	sed \
		-e 's,@VERSION@,$(VERSION),g' \
		-e 's,@PROJECT@,$(PROJECT),g' \
		-e 's,@CONFIG@,$(DESTDIR)$(sysconfdir),g' \
		-e 's,@PREFIX@,$(DESTDIR)$(datadir)/$(PROJECT),g' \
		-e 's,@BINDIR@,$(DESTDIR)$(bindir),g' \
		-e 's,@WORKDIR@,$(DESTDIR)$(workdir),g' \
		<$< >$@
	$(TOUCH_R) $< $@
	chmod --reference=$< $@

install: $(SUBDIRS) $(TARGETS) $(bin_TARGETS)
	$(MKDIR_P) -- $(DESTDIR)$(workdir)
	$(MKDIR_P) -- $(DESTDIR)$(datadir)/$(PROJECT)
	$(CP) -r -- $(DIRS) $(TARGETS) $(DESTDIR)$(datadir)/$(PROJECT)/
	$(MKDIR_P) -- $(DESTDIR)$(sysconfdir)
	$(CP) $(CONF) $(DESTDIR)$(sysconfdir)/
	$(MKDIR_P) -- $(DESTDIR)$(bindir)
	$(CP) $(bin_TARGETS) $(DESTDIR)$(bindir)/

clean: $(SUBDIRS)
	rm -f -- $(TARGETS) $(bin_TARGETS)

$(SUBDIRS):
	$(MAKE) $(MFLAGS) -C "$@" $(MAKECMDGOALS)
