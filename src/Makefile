CC   = gcc
PROG = gen_init_cpio
SRCS = gen_init_cpio.c
OBJS = $(SRCS:.c=.o)

bindir  ?= /usr/bin
DESTDIR ?=

INSTALL ?= install
MKDIR_P ?= mkdir -p

.PHONY:	all install clean

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $< $(LIBS)

install:
	$(MKDIR_P) -- $(DESTDIR)$(bindir)
	$(INSTALL) -p -m755 $(PROG) $(DESTDIR)$(bindir)/

clean:
	rm -f -- $(OBJS) $(PROG)
