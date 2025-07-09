# SPDX-License-Identifier: GPL-3.0-or-later
PHONY += guess-common

GUESS_COMMON_SCRIPTS = $(call wildcard-features,guess/common)

guess-common:
	@$(VMSG) "Processing $@ ..."
	@ $(DETECTDIR)/common/action

guess: guess-common
