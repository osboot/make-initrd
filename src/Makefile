CC   = gcc
KCC  = klcc -shared

PROG = gen_init_cpio
SRCS = gen_init_cpio.c
OBJS = $(SRCS:.c=.o)

klibc_PROGS = ls printenv replace
klibc_SRCS  = ls.c printenv.c replace.c

klibc_bindir ?= /lib/mkinitrd/klibc/bin

bindir  ?= /usr/bin
DESTDIR ?=

INSTALL ?= install
MKDIR_P ?= mkdir -p

.PHONY:	all install clean

all: $(PROG) $(klibc_PROGS)

$(PROG): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $< $(LIBS)

$(klibc_PROGS):
	$(KCC) -o $@ $@.c

install:
	$(MKDIR_P) -- $(DESTDIR)$(bindir)
	$(INSTALL) -p -m755 $(PROG) $(DESTDIR)$(bindir)/
	$(MKDIR_P) -- $(DESTDIR)$(klibc_bindir)
	$(INSTALL) -p -m755 $(klibc_PROGS) $(DESTDIR)$(klibc_bindir)/

clean:
	rm -f -- $(OBJS) $(PROG) $(klibc_PROGS)
