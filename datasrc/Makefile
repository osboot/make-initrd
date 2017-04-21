TOPDIR = $(CURDIR)/..

include $(TOPDIR)/Makefile.common

out_bindir  = _out/bin
out_sbindir = _out/sbin

CFLAGS = -Wall -Wextra -W -Wshadow -Wcast-align \
	-Wwrite-strings -Wconversion -Waggregate-return -Wstrict-prototypes \
	-Wmissing-prototypes -Wmissing-declarations -Wmissing-noreturn \
	-Wmissing-format-attribute -Wredundant-decls -Wdisabled-optimization \
	-Wno-pointer-arith

CFLAGS += -DPACKAGE=\"$@\" -DVERSION=\"$(VERSION)\" \
	-DPROGRAM_NAME=\"$(notdir $(PROG))\" \
	-D_GNU_SOURCE=1

CFLAGS += -I.
CFLAGS += -Os

bin_PROGS = \
	$(out_bindir)/halt \
	$(out_bindir)/replace \
	$(out_bindir)/showenv

sbin_PROGS = \
	$(out_sbindir)/ueventd \
	$(out_sbindir)/monotonic-timestamp

ueventd_SOURCES = \
	ueventd.c \
	uevent-epoll.c \
	uevent-logging.c \
	uevent-pidfile.c \
	uevent-worker.c

ueventd_HEADERS = \
	uevent-epoll.h \
	uevent-logging.h \
	uevent-pidfile.h

monotonic_timestamp_SOURCES = \
	monotonic-timestamp.c

halt_SOURCES = \
	halt.c

replace_SOURCES = \
	replace.c

showenv_SOURCES = \
	showenv.c

all: $(bin_PROGS) $(sbin_PROGS)

$(out_sbindir)/ueventd: $(ueventd_HEADERS) $(ueventd_SOURCES)
	$(MKDIR_P) -- $(out_sbindir)
	$(CC) $(CFLAGS) -o $@ $^

$(out_sbindir)/monotonic-timestamp: $(monotonic_timestamp_SOURCES)
	$(MKDIR_P) -- $(out_sbindir)
	$(CC) $(CFLAGS) -o $@ $^

$(out_bindir)/halt: $(halt_SOURCES)
	$(MKDIR_P) -- $(out_bindir)
	$(CC) $(CFLAGS) -o $@ $^

$(out_bindir)/replace: $(replace_SOURCES)
	$(MKDIR_P) -- $(out_bindir)
	$(CC) $(CFLAGS) -o $@ $^

$(out_bindir)/showenv: $(showenv_SOURCES)
	$(MKDIR_P) -- $(out_bindir)
	$(CC) $(CFLAGS) -o $@ $^

install: $(bin_PROGS) $(sbin_PROGS)
	$(MKDIR_P) -- $(DESTDIR)$(helper_bindir) $(DESTDIR)$(helper_sbindir)
	$(STRIP) $^
	$(INSTALL) -p -m755 $(bin_PROGS)  $(DESTDIR)$(helper_bindir)/
	$(INSTALL) -p -m755 $(sbin_PROGS) $(DESTDIR)$(helper_sbindir)/

clean:
	$(RM) -rf -- $(out_bindir) $(out_sbindir)
