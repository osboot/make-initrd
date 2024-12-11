# SPDX-License-Identifier: GPL-3.0-or-later
PHONY += guess-root

guess-root:
	$V echo "Processing $@ ..."
	@ GUESS_SUFFIX=add:$@ \
	    $(DETECTDIR)/root/action

guess: guess-root
