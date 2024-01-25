# SPDX-License-Identifier: GPL-3.0-or-later
.PHONY: put-modules

put-modules: create
	@$(VMSG) "Putting modules ..."
	@$(FEATURESDIR)/add-modules/bin/put-modules

depmod-image: put-modules
