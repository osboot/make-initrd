# SPDX-License-Identifier: GPL-3.0-or-later
MODULES_PRELOAD += dm_mod
MODULES_ADD     += dm_snapshot

PUT_UDEV_RULES += $(DM_UDEV_RULES)

PUT_FEATURE_DIRS  += $(DM_DATADIR)
PUT_FEATURE_PROGS += $(DM_PROGS)
