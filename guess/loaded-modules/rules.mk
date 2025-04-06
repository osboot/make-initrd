# SPDX-License-Identifier: GPL-3.0-or-later
PHONY += guess-loaded-modules

guess-loaded-modules:
	$V echo "Processing $@ ..."
	@ GUESS_SUFFIX=add:$@ \
	    $(DETECTDIR)/loaded-modules/action

guess: guess-loaded-modules
