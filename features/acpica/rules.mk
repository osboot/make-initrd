# SPDX-License-Identifier: GPL-3.0-or-later

PHONY += acpica
GARBAGE += $(FIRSTCPIO) $(FIRSTCPIODIR)

acpica:
	@$(VMSG) "Adding upgrade for ACPI tables ..."
	@if [ -n "$(ACPICA_AML)" ]; then \
	  mkdir -p -- "$(FIRSTCPIODIR)"/kernel/firmware/acpi; \
	  cp $(verbose3) -- $(ACPICA_AML) "$(FIRSTCPIODIR)"/kernel/firmware/acpi/; \
	fi

pack: acpica
