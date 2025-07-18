# SPDX-License-Identifier: GPL-3.0-or-later
PHONY += ucode

ucode: $(call if-active-feature,compress)
	@$(VMSG) "Adding CPU microcode ..."
	@$(FEATURESDIR)/ucode/bin/add-ucode
	@if [ -s "$(WORKDIR)/ucode.cpio" ]; then \
		cat "$(WORKDIR)/initrd.img" >> "$(WORKDIR)/ucode.cpio"; \
		mv -f $(verbose3) -- "$(WORKDIR)/ucode.cpio" "$(WORKDIR)/initrd.img"; \
	fi

install: ucode
