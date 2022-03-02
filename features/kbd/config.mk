$(call feature-requires,depmod-image)

KBD_DATADIR = $(FEATURESDIR)/kbd/data

KBD_DATA_DIR   ?= $(feature_kbd_kbddir)
KBD_FONTS_DIR   = $(KBD_DATA_DIR)/consolefonts
KBD_UNIMAPS_DIR = $(KBD_DATA_DIR)/unimaps
KBD_KEYMAPS_DIR = $(KBD_DATA_DIR)/keymaps

KBD_UTILITIES = kbd_mode setfont loadkeys

KBD_TTYS = '0 1 2 3 4 5 6 12'
KBD_UNICODE = 1

KBD_TEXTDOMAIN = kbd
