# SPDX-License-Identifier: GPL-3.0-or-later
PHONY += depmod-image

depmod-image: create
	@$(VMSG) "Generating module dependencies in image ..."
	@depmod -a $(if $(KERNEL_SYSTEM_MAP),-F "$(KERNEL_SYSTEM_MAP)") -b $(ROOTDIR) "$(KERNEL)"

pack: depmod-image
