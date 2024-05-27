# SPDX-License-Identifier: GPL-3.0-or-later

initrd_ls_DEST = $(dest_sbindir)/initrd-ls
initrd_ls_SRCS = \
	$(utils_srcdir)/initrd-ls/initrd-ls.h \
	$(utils_srcdir)/initrd-ls/initrd-ls.c \
	$(utils_srcdir)/initrd-ls/initrd-ls-format.c \
	$(utils_srcdir)/initrd-common.c \
	$(utils_srcdir)/initrd-cpio.c \
	$(utils_srcdir)/initrd-parse.c \
	$(utils_srcdir)/initrd-decompress.c \
	$(NULL)

initrd_ls_LIBS =
initrd_ls_CFLAGS += -I$(utils_srcdir)

ifeq ($(HAVE_GZIP),yes)
initrd_ls_SRCS   += $(utils_srcdir)/initrd-decompress-gzip.c
initrd_ls_LIBS   += $(HAVE_GZIP_LIBS)
initrd_ls_CFLAGS += $(HAVE_GZIP_CFLAGS)
initrd_ls_CFLAGS += -DHAVE_GZIP
else
$(warning Your system does not have zlib, disabling gzip support)
endif

ifeq ($(HAVE_BZIP2),yes)
initrd_ls_SRCS   += $(utils_srcdir)/initrd-decompress-bzip2.c
initrd_ls_LIBS   += $(HAVE_BZIP2_LIBS)
initrd_ls_CFLAGS += $(HAVE_BZIP2_CFLAGS)
initrd_ls_CFLAGS += -DHAVE_BZIP2
else
$(warning Your system does not have bzip2, disabling bzip2 support)
endif

ifeq ($(HAVE_LZMA),yes)
initrd_ls_SRCS   += $(utils_srcdir)/initrd-decompress-lzma.c
initrd_ls_LIBS   += $(HAVE_LZMA_LIBS)
initrd_ls_CFLAGS += $(HAVE_LZMA_CFLAGS)
initrd_ls_CFLAGS += -DHAVE_LZMA
else
$(warning Your system does not have liblzma, disabling lzma support)
endif

ifeq ($(HAVE_ZSTD),yes)
initrd_ls_SRCS   += $(utils_srcdir)/initrd-decompress-zstd.c
initrd_ls_LIBS   += $(HAVE_ZSTD_LIBS)
initrd_ls_CFLAGS += $(HAVE_ZSTD_CFLAGS)
initrd_ls_CFLAGS += -DHAVE_ZSTD
else
$(warning Your system does not have libzstd, disabling xz support)
endif

PROGS += initrd_ls
