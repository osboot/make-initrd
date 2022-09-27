KBD_GENERATE_CONFIG_ONCE := $(shell $(shell-export-vars) $(FEATURESDIR)/kbd/bin/get-config)

GARBAGE += \
	$(TEMPDIR)/console.sh \
	$(TEMPDIR)/console.mk

include $(TEMPDIR)/console.mk

$(call set-sysconfig,console,CONSOLE_BACKSPACE,$(CONSOLE_BACKSPACE))
$(call set-sysconfig,console,CONSOLE_FONT,$(CONSOLE_FONT))
$(call set-sysconfig,console,CONSOLE_FONT_UNIMAP,$(CONSOLE_FONT_UNIMAP))
$(call set-sysconfig,console,CONSOLE_GRP_TOGGLE,$(CONSOLE_GRP_TOGGLE))
$(call set-sysconfig,console,CONSOLE_KEYMAP,$(CONSOLE_KEYMAP))
$(call set-sysconfig,console,CONSOLE_TTYS,$(CONSOLE_TTYS))
$(call set-sysconfig,console,CONSOLE_UNICODE,$(CONSOLE_UNICODE))

KBD_DIRS    := $(shell $(shell-export-vars) $(FEATURESDIR)/kbd/bin/get-data dirs)
KBD_FILES   := $(shell $(shell-export-vars) $(FEATURESDIR)/kbd/bin/get-data files)

PUT_FEATURE_DIRS  += $(KBD_DATADIR) $(KBD_DIRS)
PUT_FEATURE_FILES += $(KBD_FILES)
PUT_FEATURE_PROGS += $(KBD_UTILITIES)

LOCALES_TEXTDOMAIN += $(KBD_TEXTDOMAIN)
