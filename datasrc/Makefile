TOPDIR = $(CURDIR)/..

include $(TOPDIR)/Makefile.common

outdir         = _out/sbin
initrdsbindir ?= /lib/initrd/sbin

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

sbin_PROGS = \
	$(outdir)/ueventd \
	$(outdir)/monotonic-timestamp

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

all: $(sbin_PROGS)

$(outdir)/ueventd: $(ueventd_HEADERS) $(ueventd_SOURCES)
	$(MKDIR_P) -- $(outdir)
	$(CC) $(CFLAGS) -o $@ $(ueventd_SOURCES)

$(outdir)/monotonic-timestamp: $(monotonic_timestamp_SOURCES)
	$(MKDIR_P) -- $(outdir)
	$(CC) $(CFLAGS) -o $@ $(monotonic_timestamp_SOURCES)

install: $(sbin_PROGS)
	$(MKDIR_P) -- $(DESTDIR)$(initrdsbindir)
	$(STRIP) $(sbin_PROGS)
	$(INSTALL) -p -m755 $(sbin_PROGS) $(DESTDIR)$(initrdsbindir)/

clean:
	$(RM) -rf -- $(outdir)
