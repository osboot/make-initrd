PROJECT = make-initrd
VERSION = $(shell sed '/^Version: */!d;s///;q' .gear/$(PROJECT).spec)

sysconfdir ?= /etc
bootdir    ?= /boot
bindir     ?= /usr/bin
sbindir    ?= /usr/sbin
libexecdir ?= /usr/libexec/$(PROJECT)
statedir   ?= /var/lib
helperdir  ?= /lib/initrd
datadir    ?= /usr/share
infodir    ?= $(datadir)/info
mandir     ?= $(datadir)/man
man1dir    ?= $(mandir)/man1
tmpdir     ?= /tmp
prefix     ?= $(datadir)/$(PROJECT)
rulesdir   ?= $(prefix)/mk

localbuilddir =

BUILDDIR ?= .build
OBJDIR = $(BUILDDIR)/obj

dest_prefix       := $(BUILDDIR)/dest$(prefix)
dest_sysconfdir   := $(BUILDDIR)/dest$(sysconfdir)
dest_bindir       := $(BUILDDIR)/dest$(bindir)
dest_sbindir      := $(BUILDDIR)/dest$(sbindir)
dest_libexecdir   := $(BUILDDIR)/dest$(libexecdir)
dest_man1dir      := $(BUILDDIR)/dest$(man1dir)
dest_rulesdir     := $(BUILDDIR)/dest$(rulesdir)
dest_datadir      := $(BUILDDIR)/dest$(helperdir)
dest_data_bindir  := $(dest_datadir)/bin
dest_data_sbindir := $(dest_datadir)/sbin

ifdef MKLOCAL
VERSION       = $(shell git describe HEAD)
prefix        = $(CURDIR)/$(dest_prefix)
sysconfdir    = $(CURDIR)/$(dest_sysconfdir)
bindir        = $(CURDIR)/$(dest_bindir)
sbindir       = $(CURDIR)/$(dest_sbindir)
libexecdir    = $(CURDIR)/$(dest_libexecdir)
rulesdir      = $(CURDIR)/$(dest_rulesdir)
localbuilddir = $(CURDIR)/$(BUILDDIR)/dest
statedir      = $(tmpdir)
endif

HAVE_GZIP    ?= $(shell pkg-config --exists zlib 2>/dev/null && echo 'yes')
HAVE_BZIP2   ?= $(shell pkg-config --exists bzip2 2>/dev/null && echo 'yes')
HAVE_LZMA    ?= $(shell pkg-config --exists liblzma 2>/dev/null && echo 'yes')
HAVE_ZSTD    ?= $(shell pkg-config --exists libzstd 2>/dev/null && echo 'yes')
HAVE_LIBKMOD ?= $(shell pkg-config --exists libkmod 2>/dev/null && echo 'yes')
HAVE_LIBELF  ?= $(shell pkg-config --exists libelf 2>/dev/null && echo 'yes')

ifneq ($(HAVE_LIBKMOD),yes)
$(error Your system does not have libkmod)
endif

VERBOSE ?= $(V)
Q = $(if $(VERBOSE),,@)
NULL :=

PROGS     =
SCRIPTS   =
C_SOURCES =

-include \
	$(wildcard utils/*/Makefile.mk) \
	$(wildcard datasrc/*/Makefile.mk)

define VAR_c_CFLAGS =
$(1)_c_CFLAGS = $(2)
endef

$(foreach name,$(PROGS),\
	$(foreach src,$($(subst -,_,$(notdir $(name)))_SRCS),\
		$(eval $(call VAR_c_CFLAGS,\
			$(basename $(subst -,_,$(notdir $(src)))),\
			$($(subst -,_,$(notdir $(name)))_CFLAGS)))))

get_objects = $(foreach name,$(filter %.c,$(1)),$(OBJDIR)/$(name:.c=.o))
get_depends = $(foreach name,$(filter %.c,$(1)),$(OBJDIR)/$(name:.c=.d))
quiet_cmd   = $(if $(VERBOSE),$(3),$(Q)printf "  %-07s%s\n" "$(1)" $(2); $(3))

PATH       = $(dest_sbindir):$(dest_bindir):$(shell echo $$PATH)
OBJS       = $(call get_objects,$(C_SOURCES))
DEPS       = $(call get_depends,$(C_SOURCES))
RULESFILES = $(addprefix $(dest_rulesdir)/,$(patsubst mk/%.in,%,$(wildcard mk/*.mk.in)))
MANPAGES   = $(addprefix $(dest_man1dir)/,$(notdir $(basename $(wildcard man/*.1.inc))))
DATAFILES  = \
	$(addprefix $(dest_prefix)/,data guess features tools kmodule.deps.d) \
	$(addprefix $(dest_sysconfdir)/,$(patsubst conf/%,%,$(wildcard conf/*)))

CFLAGS = \
	-Wall -Wextra -W -Wshadow -Wcast-align \
	-Wwrite-strings -Wconversion -Waggregate-return -Wstrict-prototypes \
	-Wmissing-prototypes -Wmissing-declarations -Wmissing-noreturn \
	-Wmissing-format-attribute -Wredundant-decls -Wdisabled-optimization \
	-Wno-pointer-arith \
	-Werror=shadow -Werror=missing-prototypes -Werror=implicit-function-declaration

CPPFLAGS = \
	-DPACKAGE=\"$(PROJECT)\" -DVERSION=\"$(VERSION)\" \
	-I$(CURDIR)/utils

CP       = $(Q)cp -a
RM       = $(Q)rm -f
LN_S     = $(Q)ln -s -f
CHMOD    = $(Q)chmod
MKDIR_P  = $(Q)mkdir -p
TOUCH_R  = $(Q)touch -r
SED      = $(call quiet_cmd,SED,$@,sed)
HELP2MAN = $(call quiet_cmd,MAN,$@,env -i PATH="${PATH}" help2man -N)
COMPILE  = $(call quiet_cmd,CC,$<,$(COMPILE.c))
LINK     = $(call quiet_cmd,CCLD,$@,$(LINK.o))
DEP      = $(call quiet_cmd,DEP,$<,$(CC))

all: build

build: build-progs build-rules build-manpages build-data

build-progs: $(OBJS) $(PROGS) $(SCRIPTS)

build-manpages: build-progs $(MANPAGES)

build-rules: $(RULESFILES)

build-data: $(DATAFILES)
	$(MKDIR_P) -- $(dest_data_bindir)
	$(LN_S) -- environ $(dest_data_bindir)/showenv

install: build-progs build-manpages
	@root="$(CURDIR)/$(BUILDDIR)/dest"; \
	find "$$root" -mindepth 1 | sort -d | while read -r n; do \
	  dst="$(DESTDIR)/$${n##$$root/}"; \
	  if [ ! -L "$$n" ] && [ -d "$$n" ]; then \
	    mkdir -p -- "$$dst"; continue; \
	  elif [ -L "$$n" ]; then \
	    t="$$(readlink "$$n")"; [ -n "$${t##$(CURDIR)/*}" ] || n="$$t"; \
	  fi; \
	  cp -a $(if $(VERBOSE),-v,) -- "$$n" "$$dst"; \
	done

clean:
	$(RM) -r -- $(BUILDDIR)

verify:
	find data features guess tools utils kmodule.deps.d -type f | \
	while read f; do \
	    ftype=$$(file -b "$$f"); \
	    [ -n "$${ftype##*shell script*}" ] || \
	    shellcheck -s dash -e SC1003,SC1090,SC1091,SC2004,SC2006,SC2015,SC2030,SC2031,SC2034,SC2086,SC2154 "$$f"; \
	done

name = $(subst -,_,$(notdir $@))
src = $($(name)_SRCS)
dst = $($(name)_DEST)
$(SCRIPTS): Makefile
	$(MKDIR_P) -- $(dir $@)
	$(SED) \
		-e 's,@PROJECT@,$(PROJECT),g' \
		-e 's,@VERSION@,$(VERSION),g' \
		-e 's,@BOOTDIR@,$(bootdir),g' \
		-e 's,@STATEDIR@,$(statedir),g' \
		-e 's,@PREFIX@,$(prefix),g' \
		-e 's,@BINDIR@,$(bindir),g' \
		-e 's,@SBINDIR@,$(sbindir),g' \
		-e 's,@LIBEXECDIR@,$(libexecdir),g' \
		-e 's,@LOCALBUILDDIR@,$(localbuilddir),g' \
		< $(src) > $(dst)
	$(TOUCH_R) $(src) $(dst)
	$(CHMOD) --reference=$(src) $(dst)

$(dest_rulesdir)/%.mk: mk/%.mk.in Makefile
	$(MKDIR_P) -- $(dir $@)
	$(SED) \
		-e 's,@PROJECT@,$(PROJECT),g' \
		-e 's,@VERSION@,$(VERSION),g' \
		-e 's,@BOOTDIR@,$(bootdir),g' \
		-e 's,@STATEDIR@,$(statedir),g' \
		-e 's,@PREFIX@,$(prefix),g' \
		-e 's,@LOCALBUILDDIR@,$(localbuilddir),g' \
		< $< > $@

$(dest_man1dir)/%.1: man/%.1.inc Makefile
	$(MKDIR_P) -- $(dir $@)
	$(HELP2MAN) -i $< -- $(notdir $*) >$@

$(dest_prefix)/%: % Makefile
	$(MKDIR_P) -- $(dir $@)
	$(RM) -- "$@"
	$(LN_S) -- "$(CURDIR)/$(notdir $@)" "$@"

$(dest_sysconfdir)/%: conf/% Makefile
	$(MKDIR_P) -- $(dir $@)
	$(CP) -- $< $@

name = $(subst -,_,$(notdir $@))
objects = $(call get_objects,$($(name)_SRCS))
$(dest_bindir)/% $(dest_sbindir)/% $(dest_libexecdir)/% $(dest_data_bindir)/% $(dest_data_sbindir)/%:
	$(MKDIR_P) -- $(dir $@)
	$(LINK) $(objects) -o $@ $($(name)_LIBS)

cflags = $($(subst -,_,$(notdir $*))_c_CFLAGS)
$(OBJDIR)/%.o: %.c Makefile
	$(MKDIR_P) -- $(dir $@)
	$(COMPILE) $(cflags) $(OUTPUT_OPTION) $<

cflags = $($(subst -,_,$(notdir $*))_c_CFLAGS)
$(OBJDIR)/%.d: %.c Makefile
	$(MKDIR_P) -- $(dir $@)
	$(DEP) -MM $(cflags) $(CPPFLAGS) $(CFLAGS) $< | \
	  sed -e 's,\($(notdir $*)\)\.o[ :]*,$(@:.d=.o) $@: Makefile ,g' >$@

ifeq ($(filter verify clean,$(MAKECMDGOALS)),)
-include $(DEPS)
endif
