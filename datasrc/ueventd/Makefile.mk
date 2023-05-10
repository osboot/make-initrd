ueventd_DEST = $(dest_data_sbindir)/ueventd
ueventd_SRCS = \
	datasrc/ueventd/memory.c \
	datasrc/ueventd/path.c \
	datasrc/ueventd/process.c \
	datasrc/ueventd/queue-processor.c \
	datasrc/ueventd/ueventd.c \
	datasrc/ueventd/ueventd.h

ueventd_CFLAGS = -D_GNU_SOURCE -Idatasrc/libinitramfs
ueventd_LIBS = -L$(dest_data_libdir) -linitramfs

PROGS += ueventd
