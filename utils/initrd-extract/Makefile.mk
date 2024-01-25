# SPDX-License-Identifier: GPL-3.0-or-later

initrd_extract_DEST = $(dest_sbindir)/initrd-extract
initrd_extract_SRCS = \
	utils/initrd-extract/initrd-extract.c \
	utils/initrd-common.c \
	utils/initrd-cpio.c \
	utils/initrd-parse.c \
	utils/initrd-decompress.c \
	$(NULL)

initrd_extract_LIBS =

ifeq ($(HAVE_GZIP),yes)
initrd_extract_SRCS   += utils/initrd-decompress-gzip.c
initrd_extract_LIBS   += $(HAVE_GZIP_LIBS)
initrd_extract_CFLAGS += $(HAVE_GZIP_CFLAGS)
initrd_extract_CFLAGS += -DHAVE_GZIP
else
$(warning Your system does not have zlib, disabling gzip support)
endif

ifeq ($(HAVE_BZIP2),yes)
initrd_extract_SRCS   += utils/initrd-decompress-bzip2.c
initrd_extract_LIBS   += $(HAVE_BZIP2_LIBS)
initrd_extract_CFLAGS += $(HAVE_BZIP2_CFLAGS)
initrd_extract_CFLAGS += -DHAVE_BZIP2
else
$(warning Your system does not have bzip2, disabling bzip2 support)
endif

ifeq ($(HAVE_LZMA),yes)
initrd_extract_SRCS   += utils/initrd-decompress-lzma.c
initrd_extract_LIBS   += $(HAVE_LZMA_LIBS)
initrd_extract_CFLAGS += $(HAVE_LZMA_CFLAGS)
initrd_extract_CFLAGS += -DHAVE_LZMA
else
$(warning Your system does not have liblzma, disabling lzma support)
endif

ifeq ($(HAVE_ZSTD),yes)
initrd_extract_SRCS   += utils/initrd-decompress-zstd.c
initrd_extract_LIBS   += $(HAVE_ZSTD_LIBS)
initrd_extract_CFLAGS += $(HAVE_ZSTD_CFLAGS)
initrd_extract_CFLAGS += -DHAVE_ZSTD
else
$(warning Your system does not have libzstd, disabling xz support)
endif

PROGS += initrd_extract
