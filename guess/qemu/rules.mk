# SPDX-License-Identifier: GPL-3.0-or-later
PHONY += guess-qemu

guess-qemu:
	@$(VMSG) "Processing $@ ..."
	@ GUESS_SUFFIX=add:$@ \
	    $(DETECTDIR)/qemu/action

guess: guess-qemu
