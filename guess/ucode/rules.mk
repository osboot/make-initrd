# SPDX-License-Identifier: GPL-3.0-or-later
PHONY += guess-ucode

guess-ucode:
	$V echo "Processing $@ ..."
	@ GUESS_SUFFIX=add:$@ \
	    $(DETECTDIR)/ucode/action

guess: guess-ucode
