md_run_DEST = $(dest_data_bindir)/md_run
md_run_SRCS = \
	datasrc/md_run/md_run.c \
	datasrc/do_mounts_md.c \
	datasrc/name_to_dev.c \
	datasrc/devname.c \
	datasrc/getarg.c \
	$(NULL)

md_run_CFLAGS = -Idatasrc

PROGS     += $(md_run_DEST)
C_SOURCES += $(md_run_SRCS)
