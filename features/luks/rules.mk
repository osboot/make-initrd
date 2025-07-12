# SPDX-License-Identifier: GPL-3.0-or-later
LUKS_CRYPTTAB_DATA =

ifneq ($(LUKS_CRYPTTAB),)
LUKS_CRYPTTAB_DATA := $(shell $(call shell-export-vars) $(FEATURESDIR)/luks/bin/get-data)
endif

MODULES_LOAD += $(LUKS_MODULES)

PUT_FEATURE_DIRS  += $(LUKS_DATADIR) $(LUKS_CRYPTTAB_DATA)
PUT_FEATURE_PROGS += $(CRYPTSETUP_BIN)
