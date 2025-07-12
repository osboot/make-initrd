# SPDX-License-Identifier: GPL-3.0-or-later
PHONY += guess-root

guess-root: prepare-guess
	@$(VMSG) "Processing $@ ..."
	@ GUESS_SUFFIX=add:$@ \
	    $(DETECTDIR)/root/action

guess: guess-root
