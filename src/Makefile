CC   = gcc

PROG = gen_init_cpio
SRCS = gen_init_cpio.c

CFLAGS = -Wall -Wextra

bindir  ?= /usr/bin
DESTDIR ?=

INSTALL ?= install
MKDIR_P ?= mkdir -p

.PHONY:	all install clean

all: $(PROG)

$(PROG):
	$(CC) $(CFLAGS) -o $@ $@.c

install: $(PROG)
	$(MKDIR_P) -- $(DESTDIR)$(bindir)
	$(INSTALL) -p -m755 $(PROG) $(DESTDIR)$(bindir)/

clean:
	rm -f -- $(OBJS) $(PROG)
