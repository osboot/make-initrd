# SPDX-License-Identifier: GPL-3.0-or-later
ifeq "$(call if-recently-activated-feature,sshfsroot)/$(GUESS_NET_IFACE)" "sshfsroot/"

GUESS_NET_IFACE = all
GUESS_MODULES +=  net

endif

GUESS_NET_IFACE ?=
