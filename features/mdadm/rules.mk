# SPDX-License-Identifier: GPL-3.0-or-later
MDADM_UDEV_RULES := $(shell $(call shell-export-vars) $(FEATURESDIR)/mdadm/bin/generate-udev-rules)
MDADM_DIRS       := $(shell $(call shell-export-vars) $(FEATURESDIR)/mdadm/bin/generate-udev-rules dirs)

PUT_UDEV_RULES    += $(MDAMD_RULES_OLD) $(MDAMD_RULES) $(MDADM_UDEV_RULES)
PUT_FEATURE_DIRS  += $(MDADM_DATADIR) $(MDADM_DIRS)
PUT_FEATURE_FILES += $(MDADM_FILES)
PUT_FEATURE_PROGS += $(MDADM_PROGS) $(MDADM_EXTRA_PROGS)
