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
initrd_extract_LIBS   += $(shell pkg-config --libs zlib)
initrd_extract_CFLAGS += $(shell pkg-config --cflags zlib)
initrd_extract_CFLAGS += -DHAVE_GZIP
else
$(warning Your system does not have zlib, disabling gzip support)
endif

ifeq ($(HAVE_BZIP2),yes)
initrd_extract_SRCS   += utils/initrd-decompress-bzip2.c
initrd_extract_LIBS   += $(shell pkg-config --libs bzip2)
initrd_extract_CFLAGS += $(shell pkg-config --cflags bzip2)
initrd_extract_CFLAGS += -DHAVE_BZIP2
else
$(warning Your system does not have bzip2, disabling bzip2 support)
endif

ifeq ($(HAVE_LZMA),yes)
initrd_extract_SRCS   += utils/initrd-decompress-lzma.c
initrd_extract_LIBS   += $(shell pkg-config --libs liblzma)
initrd_extract_CFLAGS += $(shell pkg-config --cflags liblzma)
initrd_extract_CFLAGS += -DHAVE_LZMA
else
$(warning Your system does not have liblzma, disabling lzma support)
endif

ifeq ($(HAVE_ZSTD),yes)
initrd_extract_SRCS   += utils/initrd-decompress-zstd.c
initrd_extract_LIBS   += $(shell pkg-config --libs libzstd)
initrd_extract_CFLAGS += $(shell pkg-config --cflags libzstd)
initrd_extract_CFLAGS += -DHAVE_ZSTD
else
$(warning Your system does not have libzstd, disabling xz support)
endif

PROGS     += $(initrd_extract_DEST)
C_SOURCES += $(initrd_extract_SRCS)
