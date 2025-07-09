# SPDX-License-Identifier: GPL-3.0-or-later
PHONY += guess-lkrg

guess-lkrg:
	@$(VMSG) "Processing $@ ..."
	@ GUESS_SUFFIX=add:$@ \
	    $(DETECTDIR)/lkrg/action

guess: guess-lkrg
