# SPDX-License-Identifier: GPL-3.0-or-later

MODULES_LOAD += $(LUKS_MODULES)

PUT_FEATURE_DIRS  += $(LUKS_DATADIR)
PUT_FEATURE_PROGS += $(CRYPTSETUP_BIN)

ifneq ($(LUKS_CRYPTTAB),)
luks-crypttab: create
	@$(VMSG) "Adding LUKS crypttab..."
	@$(FEATURESDIR)/luks/bin/get-data

pack: luks-crypttab
endif
