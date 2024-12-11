# SPDX-License-Identifier: GPL-3.0-or-later
PHONY += guess-bootconfig

guess-bootconfig:
	$V echo "Processing $@ ..."
	@ GUESS_SUFFIX=add:$@ \
	    $(DETECTDIR)/bootconfig/action

guess: guess-bootconfig
