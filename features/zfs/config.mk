# SPDX-License-Identifier: GPL-3.0-or-later
$(call feature-requires,add-modules depmod-image)

ZFS_DATADIR = $(FEATURESDIR)/zfs/data
ZFS_PROGS   = zfs zpool zgenhostid mount.zfs
ZFS_PRELOAD = zfs
ZFS_RULES   = *-zvol.rules *-vdev.rules *-zfs.rules
