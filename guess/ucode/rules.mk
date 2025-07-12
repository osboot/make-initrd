# SPDX-License-Identifier: GPL-3.0-or-later
PHONY += guess-ucode

guess-ucode: prepare-guess
	@$(VMSG) "Processing $@ ..."
	@ GUESS_SUFFIX=add:$@ \
	    $(DETECTDIR)/ucode/action

guess: guess-ucode
