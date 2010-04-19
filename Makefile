PROJECT = make-initrd
VERSION = $(shell sed '/^Version: */!d;s///;q' make-initrd.spec)

sysconfdir ?= /etc
bootdir    ?= /boot
bindir     ?= /usr/bin
sbindir    ?= /usr/sbin
datadir    ?= /usr/share
mandir     ?= $(datadir)/man
man1dir    ?= $(mandir)/man1
tmpdir     ?= /tmp
prefix     ?= $(datadir)/$(PROJECT)
DESTDIR    ?=

ifdef MKLOCAL
prefix  = $(CURDIR)
bindir  = $(CURDIR)
sbindir = $(CURDIR)
endif

MAN1PAGES = make-initrd.1
MANPAGES  = $(MAN1PAGES)

TEXIDOCS  = make-initrd.texi
INFODOCS  = $(TEXIDOCS:.texi=.info)

CP = cp -a
INSTALL = install
MKDIR_P = mkdir -p
TOUCH_R = touch -r
HELP2MAN = env -i help2man -N
MAKEINFO_FLAGS = -D "VERSION $(VERSION)"

DIRS = data features tools

CONF = initrd.mk

PREPROCESS_TARGET = make-initrd mkinitrd-make-initrd config.mk make-initrd.mk

sbin_TARGETS = make-initrd mkinitrd-make-initrd

TARGETS = config.mk rules.mk initfiles.mk make-initrd.mk

SUBDIRS = src

.PHONY: $(SUBDIRS)

all: $(SUBDIRS) $(TARGETS) $(sbin_TARGETS) $(MANPAGES) $(INFODOCS)

%.1: % %.1.inc
	$(HELP2MAN) -i $@.inc -- ./$< >$@

%: %.in
	sed \
		-e 's,@VERSION@,$(VERSION),g' \
		-e 's,@PROJECT@,$(PROJECT),g' \
		-e 's,@BOOTDIR@,$(bootdir),g' \
		-e 's,@CONFIG@,$(sysconfdir),g' \
		-e 's,@PREFIX@,$(prefix),g' \
		-e 's,@BINDIR@,$(bindir),g' \
		-e 's,@SBINDIR@,$(sbindir),g' \
		-e 's,@TMPDIR@,$(tmpdir),g' \
		<$< >$@
	$(TOUCH_R) $< $@
	chmod --reference=$< $@

install: $(SUBDIRS) $(TARGETS) $(sbin_TARGETS)
	$(MKDIR_P) -- $(DESTDIR)$(tempdir)
	$(MKDIR_P) -- $(DESTDIR)$(datadir)/$(PROJECT)
	$(CP) -r -- $(DIRS) $(TARGETS) $(DESTDIR)$(datadir)/$(PROJECT)/
	$(MKDIR_P) -- $(DESTDIR)$(sysconfdir)/initrd.mk.d
	$(CP) $(CONF) $(DESTDIR)$(sysconfdir)/
	$(MKDIR_P) -- $(DESTDIR)$(sbindir)
	$(CP) $(sbin_TARGETS) $(DESTDIR)$(sbindir)/
	$(MKDIR_P) -- $(DESTDIR)$(man1dir)
	$(CP) $(MAN1PAGES) $(DESTDIR)$(man1dir)/

clean: $(SUBDIRS)
	rm -f -- $(PREPROCESS_TARGET) $(MANPAGES) $(INFODOCS)

$(SUBDIRS):
	$(MAKE) $(MFLAGS) -C "$@" $(MAKECMDGOALS)
