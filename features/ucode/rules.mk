# SPDX-License-Identifier: GPL-3.0-or-later
PHONY += ucode

GARBAGE += $(FIRSTCPIO) $(FIRSTCPIODIR)

ucode:
	@$(VMSG) "Adding CPU microcode ..."
	@$(FEATURESDIR)/ucode/bin/add-ucode

pack: ucode
