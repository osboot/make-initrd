$(call feature-requires,depmod-image)

ZFS_DATADIR = $(FEATURESDIR)/zfs/data
ZFS_PROGS   = zfs zpool zgenhostid mount.zfs
ZFS_PRELOAD = zfs
ZFS_RULES   = *-zvol.rules *-vdev.rules *-zfs.rules
