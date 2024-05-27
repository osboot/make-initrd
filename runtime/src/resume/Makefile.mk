# SPDX-License-Identifier: GPL-3.0-or-later

resume_DEST = $(dest_data_bindir)/resume
resume_SRCS = \
	$(runtime_srcdir)/resume/resume.h \
	$(runtime_srcdir)/resume/resume.c \
	$(runtime_srcdir)/resume/resumelib.c \
	$(runtime_srcdir)/devname.c \
	$(runtime_srcdir)/getarg.c \
	$(runtime_srcdir)/name_to_dev.c \
	$(runtime_srcdir)/do_mounts.h \
	$(runtime_srcdir)/kinit.h \
	$(NULL)

resume_CFLAGS = -I$(runtime_srcdir)

PROGS += resume
