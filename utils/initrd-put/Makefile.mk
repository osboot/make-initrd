# SPDX-License-Identifier: GPL-3.0-or-later

ifeq ($(HAVE_LIBELF),yes)
initrd_put_DEST = $(dest_bindir)/initrd-put
initrd_put_SRCS = $(utils_srcdir)/initrd-put/initrd-put.c
initrd_put_LIBS = $(HAVE_LIBELF_LIBS)
initrd_put_CFLAGS = $(HAVE_LIBELF_CFLAGS) \
		    -DPACKAGE_VERSION=\"$(PACKAGE_VERSION)\"

PROGS += initrd_put

else
$(error Your system does not have libelf, disabling initrd-put)
endif
