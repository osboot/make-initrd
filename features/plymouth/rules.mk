# SPDX-License-Identifier: GPL-3.0-or-later
PLYMOUTH_DIRS := $(foreach d,$(LIB_DIRS),$(wildcard $(d)/plymouth))

$(call assgin-shell-once,PLYMOUTH_PACK_MODULES, $(FEATURESDIR)/plymouth/bin/get-modules)
$(call assgin-shell-once,PLYMOUTH_PACK_THEME,   $(FEATURESDIR)/plymouth/bin/get-theme-files "$(PLYMOUTH_THEME)
$(call assgin-shell-once,PLYMOUTH_PACK_DEFFONT, fc-match -f '%{file}\n' 'DejaVuSans' ||:)

PLYMOUTH_PACK_FONT := $(or $(PLYMOUTH_FONT),$(PLYMOUTH_PACK_DEFFONT))

PLYMOUTH_EXCLUDE_MODULES   ?=
PLYMOUTH_EXCLUDE_RENDERERS ?= %/x11.so

PLYMOUTH_PACK_FILES ?= \
	$(wildcard $(SYSCONFDIR)/plymouth/plymouthd.conf) \
	$(wildcard $(SYSCONFDIR)/*-release) \
	$(wildcard /var/lib/plymouth/boot-duration) \
	$(wildcard $(DATADIR)/plymouth/plymouthd.defaults) \
	$(filter-out $(PLYMOUTH_EXCLUDE_MODULES),$(foreach d,$(PLYMOUTH_DIRS),$(wildcard $(d)/*.so))) \
	$(filter-out $(PLYMOUTH_EXCLUDE_RENDERERS),$(foreach d,$(PLYMOUTH_DIRS),$(wildcard $(d)/renderers/*.so))) \
	$(DATADIR)/plymouth/themes/details/details.plymouth \
	$(DATADIR)/plymouth/themes/text/text.plymouth \
	$(PLYMOUTH_PACK_FONT) \
	$(PLYMOUTH_PACK_THEME)

MODULES_ADD += $(PLYMOUTH_PACK_MODULES)
MODULES_TRY_ADD += drivers/char/agp

PUT_UDEV_RULES    += $(PLYMOUTH_UDEV_RULES)
PUT_FEATURE_DIRS  += $(PLYMOUTH_DATADIR)
PUT_FEATURE_FILES += $(PLYMOUTH_PACK_FILES) $(PLYMOUTH_FILES)
PUT_FEATURE_PROGS += $(PLYMOUTH_PROGS)

LOCALES_TEXTDOMAIN += $(PLYMOUTH_TEXTDOMAIN)
