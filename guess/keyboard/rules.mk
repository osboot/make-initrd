# SPDX-License-Identifier: GPL-3.0-or-later
PHONY += guess-keyboard

guess-keyboard: prepare-guess
	@$(VMSG) "Processing $@ ..."
	@ GUESS_SUFFIX=rescue:$@ \
	    $(DETECTDIR)/keyboard/action

guess: guess-keyboard
