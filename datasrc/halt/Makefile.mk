halt_DEST = $(dest_data_bindir)/halt
halt_SRCS = \
	datasrc/halt/halt.c \
	datasrc/halt/hddown.c \
	datasrc/halt/ifdown.c \
	datasrc/halt/utmp.c \
	datasrc/halt/reboot.h

PROGS += halt
