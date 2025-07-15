# SPDX-License-Identifier: GPL-3.0-or-later

MODULES_PRELOAD += $(ZFS_PRELOAD)

PUT_FEATURE_DIRS  += $(ZFS_DATADIR)
PUT_FEATURE_PROGS += $(ZFS_PROGS)
PUT_UDEV_RULES    += $(ZFS_RULES)

zfs: create
	@$(VMSG) "Adding zfs data..."
	@$(FEATURESDIR)/zfs/bin/get-data

pack: zfs
