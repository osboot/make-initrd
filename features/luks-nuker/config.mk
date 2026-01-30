# SPDX-License-Identifier: GPL-3.0-or-later

$(call feature-requires, luks, dialog)

LUKS_NUKER_CONF ?= $(SYSCONFDIR)/luks-nuker.conf

LUKS_NUKER_PROGS = luks-nuker
LUKS_NUKER_FILES = $(LUKS_NUKER_CONF)

LUKS_NUKER_ARGS ?=
