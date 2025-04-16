# SPDX-License-Identifier: GPL-3.0-or-later

PHONY += create

PUT_FEATURE_DIRS += $(FEATURESDIR)/runtime/data

create: depmod-host
	@$(VMSG) "Creating initrd image ..."
	@mkdir -m 755 -p $(verbose) -- $(ROOTDIR)
	@$(TOOLSDIR)/create-initrd
