# SPDX-License-Identifier: GPL-3.0-or-later

SHUTDOWN_FEATURE_DIRS := $(wildcard $(foreach f,$(ALL_ACTIVE_FEATURES),$(FEATURESDIR)/$(f)/shutdown))

PUT_FEATURE_DIRS += $(SHUTDOWN_FEATURE_DIRS) $(SHUTDOWN_DIRS)

PUT_FEATURE_PROGS += fuser stat sync timeout losetup
