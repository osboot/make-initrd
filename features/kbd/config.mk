KBD_DATADIR = $(FEATURESDIR)/kbd/data

KBD_FONTS_DIR   = /lib/kbd/consolefonts
KBD_KEYMAPS_DIR = /lib/kbd/keymaps

KBD_FILES = \
	/bin/kbd_mode \
	/bin/setfont \
	/bin/loadkeys \
	$(KBD_KEYMAPS_DIR)/i386 \
	$(KBD_KEYMAPS_DIR)/include \
	$(KBD_FONTS_DIR)

KBD_CONFIGS = \
	/etc/sysconfig/consolefont \
	/etc/sysconfig/keyboard
