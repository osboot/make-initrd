# SPDX-License-Identifier: GPL-3.0-or-later
ifeq "$(call if-recently-activated-feature,sshfsroot)" "sshfsroot"
GUESS_MODULES += smart-card
endif
