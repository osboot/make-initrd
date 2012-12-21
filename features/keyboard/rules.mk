KEYBOARD_MODLUES_DETECTED = $(shell [ ! -s $(GUESSDIR)/kbdmodules ] || cat $(GUESSDIR)/kbdmodules)

ifeq "$(KEYBOARD_MODLUES_DETECTED)" ''
KEYBOARD_MODLUES_DETECTED = $(KEYBOARD_DEFAULT_MODULES)
endif

keyboard: create
	@echo "Adding usb keyboard support ..."
	@[ -z "$(KEYBOARD_MODLUES_DETECTED)" ] || \
	   $(TOOLSDIR)/add-rescue-modules $(KEYBOARD_MODLUES_DETECTED);

pack: keyboard
depmod-image: keyboard
