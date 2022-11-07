PLYMOUTH_LIBDIR := $(shell $(call shell-export-vars) $(FEATURESDIR)/plymouth/bin/get-libdir)

PLYMOUTH_PACK_MODULES := $(shell $(call shell-export-vars) $(FEATURESDIR)/plymouth/bin/get-modules)
PLYMOUTH_PACK_FONT    := $(or $(PLYMOUTH_FONT),$(shell fc-match -f '%{file}\n' 'DejaVuSans' ||:))
PLYMOUTH_PACK_THEME   := $(shell $(call shell-export-vars) $(FEATURESDIR)/plymouth/bin/get-theme-files "$(PLYMOUTH_THEME)")

PLYMOUTH_PACK_FILES ?= \
	$(wildcard $(SYSCONFDIR)/plymouth/plymouthd.conf) \
	$(wildcard $(SYSCONFDIR)/*-release) \
	$(wildcard /var/lib/plymouth/boot-duration) \
	$(wildcard $(DATADIR)/plymouth/plymouthd.defaults) \
	$(DATADIR)/plymouth/themes/details/details.plymouth \
	$(DATADIR)/plymouth/themes/text/text.plymouth \
	$(PLYMOUTH_LIBDIR)/details.so \
	$(PLYMOUTH_LIBDIR)/text.so \
	$(PLYMOUTH_LIBDIR)/label.so \
	$(PLYMOUTH_LIBDIR)/renderers/drm.so \
	$(PLYMOUTH_LIBDIR)/renderers/frame-buffer.so \
	$(PLYMOUTH_PACK_FONT) \
	$(PLYMOUTH_PACK_THEME)

MODULES_ADD += $(PLYMOUTH_PACK_MODULES)
MODULES_TRY_ADD += drivers/char/agp

PUT_FEATURE_DIRS  += $(PLYMOUTH_DATADIR)
PUT_FEATURE_FILES += $(PLYMOUTH_PACK_FILES) $(PLYMOUTH_FILES)
PUT_FEATURE_PROGS += $(PLYMOUTH_PROGS)

LOCALES_TEXTDOMAIN += $(PLYMOUTH_TEXTDOMAIN)
