# SPDX-License-Identifier: GPL-3.0-or-later
PHONY += guess-device-tree

guess-device-tree: prepare-guess
	@$(VMSG) "Processing $@ ..."
	@ GUESS_SUFFIX=add:$@ \
	    $(DETECTDIR)/device-tree/action

guess: guess-device-tree
