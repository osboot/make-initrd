bootmenu_DEST = $(dest_featuredir)/bootloader/bin/bootmenu
bootmenu_SRCS = $(FEATURESDIR)/bootloader/src/bootmenu/bootmenu.c
bootmenu_LIBS = -lnewt -lslang -liniparser 

PROGS += bootmenu
