# SPDX-License-Identifier: GPL-3.0-or-later

PUT_FEATURE_FILES += $(LUKS_NUKER_FILES)
PUT_FEATURE_PROGS += $(LUKS_NUKER_PROGS)

$(call set-sysconfig,luks-nuker,LUKS_NUKER_ARGS,$(LUKS_NUKER_ARGS))
