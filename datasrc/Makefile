TOPDIR = $(CURDIR)/..

include $(TOPDIR)/Makefile.common

outdir   = _out/sbin
sbindir ?= /lib/initrd/sbin

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

PROG = $(outdir)/ueventd

SOURCES = \
	ueventd.c \
	uevent-epoll.c \
	uevent-logging.c \
	uevent-pidfile.c \
	uevent-worker.c

HEADERS = \
	uevent-epoll.h \
	uevent-logging.h \
	uevent-pidfile.h

all: $(PROG)

$(PROG): $(HEADERS) $(SOURCES)
	$(MKDIR_P) -- $(outdir)
	$(CC) $(CFLAGS) -o $(PROG) $(SOURCES)

install: $(PROG)
	$(MKDIR_P) -- $(DESTDIR)$(sbindir)
	$(STRIP) $(PROG)
	$(INSTALL) -p -m755 $(PROG) $(DESTDIR)$(sbindir)/

clean:
	$(RM) -rf -- $(outdir)
