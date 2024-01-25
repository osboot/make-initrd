# SPDX-License-Identifier: GPL-3.0-or-later
ZFS_DATA := $(shell $(call shell-export-vars) $(FEATURESDIR)/zfs/bin/get-data)

MODULES_PRELOAD += $(ZFS_PRELOAD)

PUT_FEATURE_DIRS  += $(ZFS_DATADIR) $(ZFS_DATA)
PUT_FEATURE_PROGS += $(ZFS_PROGS)
PUT_UDEV_RULES    += $(ZFS_RULES)

