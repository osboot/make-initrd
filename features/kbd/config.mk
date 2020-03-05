KBD_DATADIR = $(FEATURESDIR)/kbd/data

KBD_DATA_DIR   ?= /lib/kbd
KBD_FONTS_DIR   = $(KBD_DATA_DIR)/consolefonts
KBD_UNIMAPS_DIR = $(KBD_DATA_DIR)/unimaps
KBD_KEYMAPS_DIR = $(KBD_DATA_DIR)/keymaps

KBD_FILES = \
	/bin/kbd_mode \
	/bin/setfont \
	/bin/loadkeys

KBD_TTYS = '0 1 2 3 4 5 6 12'
KBD_UNICODE = 1
