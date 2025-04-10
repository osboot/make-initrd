# SPDX-License-Identifier: GPL-3.0-or-later

ifeq ($(HAVE_LIBELF),yes)
initrd_put_DEST = $(dest_bindir)/initrd-put
initrd_put_SRCS = \
	$(utils_srcdir)/initrd-put/memory.c \
	$(utils_srcdir)/initrd-put/queue.c \
	$(utils_srcdir)/initrd-put/tree.c \
	$(utils_srcdir)/initrd-put/enqueue-library.c \
	$(utils_srcdir)/initrd-put/enqueue-shebang.c \
	$(utils_srcdir)/initrd-put/initrd-put.c

initrd_put_LIBS = $(fts_LIBS) $(HAVE_LIBELF_LIBS) $(HAVE_LIBJSON_C_LIBS)
initrd_put_CFLAGS = $(HAVE_LIBELF_CFLAGS) $(HAVE_LIBJSON_C_CFLAGS) \
		    -I$(utils_srcdir)/initrd-put \
		    -DPACKAGE_VERSION=\"$(PACKAGE_VERSION)\"

ifeq ($(HAVE_LIBJSON_C),yes)
initrd_put_SRCS += $(utils_srcdir)/initrd-put/elf_dlopen.c
initrd_put_CFLAGS += -DHAVE_LIBJSON_C
endif

PROGS += initrd_put

else
$(error Your system does not have libelf, disabling initrd-put)
endif
