PROJECT = make-initrd
VERSION = $(shell sed '/^Version: */!d;s///;q' make-initrd.spec)

sysconfdir ?= /etc
bindir     ?= /usr/bin
sbindir    ?= /usr/sbin
datadir    ?= /usr/share
workdir    ?= /tmp
prefix     ?= $(datadir)/$(PROJECT)
DESTDIR    ?=

ifdef MKLOCAL
prefix  = $(CURDIR)
bindir  = $(CURDIR)
sbindir = $(CURDIR)
endif

CP = cp -a
INSTALL = install
MKDIR_P = mkdir -p
TOUCH_R = touch -r

DIRS = data features tools

CONF = initrd.mk

sbin_TARGETS = make-initrd mkinitrd-make-initrd

TARGETS = config.mk rules.mk initfiles.mk

SUBDIRS = src

.PHONY: $(SUBDIRS)

all: $(SUBDIRS) $(TARGETS) $(sbin_TARGETS)

%: %.in
	sed \
		-e 's,@VERSION@,$(VERSION),g' \
		-e 's,@PROJECT@,$(PROJECT),g' \
		-e 's,@CONFIG@,$(DESTDIR)$(sysconfdir),g' \
		-e 's,@PREFIX@,$(DESTDIR)$(prefix),g' \
		-e 's,@BINDIR@,$(DESTDIR)$(bindir),g' \
		-e 's,@SBINDIR@,$(DESTDIR)$(sbindir),g' \
		-e 's,@WORKDIR@,$(DESTDIR)$(workdir),g' \
		<$< >$@
	$(TOUCH_R) $< $@
	chmod --reference=$< $@

install: $(SUBDIRS) $(TARGETS) $(sbin_TARGETS)
	$(MKDIR_P) -- $(DESTDIR)$(workdir)
	$(MKDIR_P) -- $(DESTDIR)$(datadir)/$(PROJECT)
	$(CP) -r -- $(DIRS) $(TARGETS) $(DESTDIR)$(datadir)/$(PROJECT)/
	$(MKDIR_P) -- $(DESTDIR)$(sysconfdir)/initrd.mk.d
	$(CP) $(CONF) $(DESTDIR)$(sysconfdir)/
	$(MKDIR_P) -- $(DESTDIR)$(sbindir)
	$(CP) $(sbin_TARGETS) $(DESTDIR)$(sbindir)/

clean: $(SUBDIRS)
	rm -f -- $(TARGETS) $(sbin_TARGETS)

$(SUBDIRS):
	$(MAKE) $(MFLAGS) -C "$@" $(MAKECMDGOALS)
