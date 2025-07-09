# SPDX-License-Identifier: GPL-3.0-or-later
PHONY += guess-touchscreen

guess-touchscreen:
	@$(VMSG) "Processing $@ ..."
	@ GUESS_SUFFIX=rescue:$@ \
	    $(DETECTDIR)/touchscreen/action

guess: guess-touchscreen
