bootmenu_DEST = $(dest_featuredir)/bootloader/bin/bootmenu
bootmenu_SRCS = $(FEATURESDIR)/bootloader/src/bootmenu/bootmenu.c
bootmenu_LIBS = $(HAVE_LIBNEWT_LIBS) $(HAVE_LIBSLANG_LIBS) $(HAVE_LIBINIPARSER_LIBS)
bootmenu_CFLAGS = $(HAVE_LIBNEWT_CFLAGS) $(HAVE_LIBSLANG_CFLAGS) $(HAVE_LIBINIPARSER_CFLAGS)

PROGS += bootmenu
