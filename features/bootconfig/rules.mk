# SPDX-License-Identifier: GPL-3.0-or-later
PHONY += bootconfig

bootconfig: $(call if-feature,compress)
	@$(VMSG) "Adding bootconfig ..."
	@$(FEATURESDIR)/bootconfig/bin/add-bootconfig

install: bootconfig
