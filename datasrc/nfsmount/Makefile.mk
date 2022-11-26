nfsmount_DEST = $(dest_data_bindir)/nfsmount
nfsmount_SRCS = \
	datasrc/nfsmount/dummypmap.c \
	datasrc/nfsmount/dummypmap.h \
	datasrc/nfsmount/main.c \
	datasrc/nfsmount/mount.c \
	datasrc/nfsmount/nfsmount.h \
	datasrc/nfsmount/portmap.c \
	datasrc/nfsmount/sunrpc.c \
	datasrc/nfsmount/sunrpc.h \
	$(NULL)

nfsmount_CFLAGS = -Idatasrc -D_GNU_SOURCE=1 -Wno-conversion -Wno-sign-conversion -Wno-stringop-truncation

PROGS += nfsmount
