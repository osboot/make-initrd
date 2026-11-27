# SPDX-License-Identifier: GPL-3.0-or-later

initrd_runtime_SRCS += \
	$(runtime_srcdir)/resume/resume.h \
	$(runtime_srcdir)/resume/resume.c \
	$(runtime_srcdir)/resume/resumelib.c \
	$(runtime_srcdir)/resume/getarg.c \
	$(runtime_srcdir)/resume/name_to_dev.c \
	$(runtime_srcdir)/resume/name_to_dev.h \
	$(NULL)

initrd_runtime_SYMLINKS += $(dest_data_bindir)/resume
