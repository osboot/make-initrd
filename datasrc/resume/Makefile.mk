resume_DEST = $(dest_data_bindir)/resume
resume_SRCS = \
	datasrc/resume/resume.h \
	datasrc/resume/resume.c \
	datasrc/resume/resumelib.c \
	datasrc/devname.c \
	datasrc/getarg.c \
	datasrc/name_to_dev.c \
	datasrc/do_mounts.h \
	datasrc/kinit.h \
	$(NULL)

resume_CFLAGS = -Idatasrc

PROGS     += $(resume_DEST)
C_SOURCES += $(resume_SRCS)
