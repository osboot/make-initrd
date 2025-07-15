# SPDX-License-Identifier: GPL-3.0-or-later

$(call assgin-shell-once,MDADM_UDEV_RULES,$(FEATURESDIR)/mdadm/bin/generate-udev-rules)
$(call assgin-shell-once,MDADM_DIRS,      $(FEATURESDIR)/mdadm/bin/generate-udev-rules dirs)

PUT_UDEV_RULES    += $(MDAMD_RULES_OLD) $(MDAMD_RULES) $(MDADM_UDEV_RULES)
PUT_FEATURE_DIRS  += $(MDADM_DATADIR) $(MDADM_DIRS)
PUT_FEATURE_FILES += $(MDADM_FILES)
PUT_FEATURE_PROGS += $(MDADM_PROGS) $(MDADM_EXTRA_PROGS)
