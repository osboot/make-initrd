# SPDX-License-Identifier: GPL-3.0-or-later
PHONY += guess-device

guess-device:
	$V echo "Processing $@ ..."
	@ GUESS_SUFFIX=add:$@ \
	    $(DETECTDIR)/device/action

guess: guess-device
