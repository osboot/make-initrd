# SPDX-License-Identifier: GPL-3.0-or-later
.PHONY: guess-net

guess-net:
	$V echo "Processing $@ ..."
	@ GUESS_SUFFIX=add:$@ \
	    $(DETECTDIR)/net/action

guess: guess-net
