initrd_ls_DEST = $(dest_sbindir)/initrd-ls
initrd_ls_SRCS = \
	utils/initrd-ls/initrd-ls.h \
	utils/initrd-ls/initrd-ls.c \
	utils/initrd-ls/initrd-ls-format.c \
	utils/initrd-common.c \
	utils/initrd-cpio.c \
	utils/initrd-parse.c \
	utils/initrd-decompress.c \
	$(NULL)

initrd_ls_LIBS =

ifeq ($(HAVE_GZIP),yes)
initrd_ls_SRCS   += utils/initrd-decompress-gzip.c
initrd_ls_LIBS   += $(shell pkg-config --libs zlib)
initrd_ls_CFLAGS += $(shell pkg-config --cflags zlib)
initrd_ls_CFLAGS += -DHAVE_GZIP
else
$(warning Your system does not have zlib, disabling gzip support)
endif

ifeq ($(HAVE_BZIP2),yes)
initrd_ls_SRCS   += utils/initrd-decompress-bzip2.c
initrd_ls_LIBS   += $(shell pkg-config --libs bzip2)
initrd_ls_CFLAGS += $(shell pkg-config --cflags bzip2)
initrd_ls_CFLAGS += -DHAVE_BZIP2
else
$(warning Your system does not have bzip2, disabling bzip2 support)
endif

ifeq ($(HAVE_LZMA),yes)
initrd_ls_SRCS   += utils/initrd-decompress-lzma.c
initrd_ls_LIBS   += $(shell pkg-config --libs liblzma)
initrd_ls_CFLAGS += $(shell pkg-config --cflags liblzma)
initrd_ls_CFLAGS += -DHAVE_LZMA
else
$(warning Your system does not have liblzma, disabling lzma support)
endif

ifeq ($(HAVE_ZSTD),yes)
initrd_ls_SRCS   += utils/initrd-decompress-zstd.c
initrd_ls_LIBS   += $(shell pkg-config --libs libzstd)
initrd_ls_CFLAGS += $(shell pkg-config --cflags libzstd)
initrd_ls_CFLAGS += -DHAVE_ZSTD
else
$(warning Your system does not have libzstd, disabling xz support)
endif

PROGS     += $(initrd_ls_DEST)
C_SOURCES += $(initrd_ls_SRCS)
