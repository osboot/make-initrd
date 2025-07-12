# SPDX-License-Identifier: GPL-3.0-or-later
PHONY += guess-smart-card

guess-smart-card: prepare-guess
	@$(VMSG) "Processing $@ ..."
	@ GUESS_SUFFIX=add:$@ \
	    $(DETECTDIR)/smart-card/action

guess: guess-smart-card
