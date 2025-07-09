# SPDX-License-Identifier: GPL-3.0-or-later
PHONY += depmod-image

depmod-image: create
	@$(VMSG) "Generating module dependencies in image ..."
	@system_map_file="$(KERNEL_SYSTEM_MAP)"; \
	[ -e "$$system_map_file" ] || system_map_file=""; \
	/sbin/depmod -a $${system_map_file:+-F "$$system_map_file"} -b $(ROOTDIR) "$(KERNEL)"

pack: depmod-image
