environ_DEST = $(dest_data_bindir)/environ
environ_SRCS = datasrc/environ/environ.c

PROGS += environ

environ_CFLAGS = -D_GNU_SOURCE -Idatasrc/libinitramfs
environ_LIBS = -L$(dest_data_libdir) -linitramfs

environ_SYMLINKS += $(dest_data_bindir)/showenv
