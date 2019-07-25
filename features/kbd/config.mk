KBD_DATADIR = $(FEATURESDIR)/kbd/data

KBD_DATA_DIR   ?= /lib/kbd
KBD_FONTS_DIR   = $(KBD_DATA_DIR)/consolefonts
KBD_UNIMAPS_DIR = $(KBD_DATA_DIR)/unimaps
KBD_KEYMAPS_DIR = $(KBD_DATA_DIR)/keymaps

KBD_FILES = \
	/bin/kbd_mode \
	/bin/setfont \
	/bin/loadkeys \
	$(wildcard /usr/[s]bin/fbset)

KBD_FRAMEBUFFER_MODULE = uvesafb
KBD_FRAMEBUFFER_MODE   = 800x600-80
