environ_DEST = $(dest_data_bindir)/environ
environ_SRCS = datasrc/environ/environ.c

PROGS += environ

environ_SYMLINKS += $(dest_data_bindir)/showenv
