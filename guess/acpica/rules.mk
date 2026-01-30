# SPDX-License-Identifier: GPL-3.0-or-later
PHONY += guess-acpica

guess-acpica: prepare-guess
	@$(VMSG) "Processing $@ ..."
	@ GUESS_SUFFIX=add:$@ \
	    $(DETECTDIR)/acpica/action

guess: guess-acpica
