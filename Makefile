PROJECT = make-initrd
VERSION = 0.1.0

bindir  = /usr/bin
datadir = /usr/share
workdir = /tmp
DESTDIR ?=

CP = cp -a
INSTALL = install
MKDIR_P = mkdir -p
TOUCH_R = touch -r

DIRS = autodetect data features tools

TARGETS = config.mk rules.mk

SUBDIRS = src

.PHONY:	$(SUBDIRS)

all: $(SUBDIRS) $(TARGETS)

%: %.in
	sed \
		-e 's,@VERSION@,$(VERSION),g' \
		-e 's,@PROJECT@,$(PROJECT),g' \
		-e 's,@PREFIX@,$(DESTDIR)$(datadir)/$(PROJECT),g' \
		-e 's,@BINDIR@,$(DESTDIR)$(bindir),g' \
		-e 's,@WORKDIR@,$(DESTDIR)$(workdir),g' \
		<$< >$@
	$(TOUCH_R) $< $@
	chmod --reference=$< $@

install: $(SUBDIRS) $(TARGETS)
	$(MKDIR_P) -- $(DESTDIR)$(datadir)/$(PROJECT) $(DESTDIR)$(workdir)
	$(CP) -r -- $(DIRS) $(TARGETS)  $(DESTDIR)$(datadir)/$(PROJECT)/

clean: $(SUBDIRS)
	rm -f -- $(TARGETS)

$(SUBDIRS):
	$(MAKE) $(MFLAGS) -C "$@" $(MAKECMDGOALS)
