procacct_DEST = $(dest_data_bindir)/procacct
procacct_SRCS = $(FEATURESDIR)/debug-procacct/src/procacct.c
procacct_CFLAGS = -D_GNU_SOURCE -Idatasrc/libinitramfs
procacct_LIBS = -L$(dest_data_libdir) -linitramfs

PROGS += procacct
