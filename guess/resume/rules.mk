# SPDX-License-Identifier: GPL-3.0-or-later
PHONY += guess-resume

guess-resume:
	$V echo "Processing $@ ..."
	@ GUESS_SUFFIX=add:$@ \
	    $(DETECTDIR)/resume/action

guess: guess-resume
