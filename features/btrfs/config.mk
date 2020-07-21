$(call feature-requires,depmod-image)

BTRFS_DATADIR = $(FEATURESDIR)/btrfs/data
BTRFS_PROGS   = btrfs
BTRFS_PRELOAD = btrfs
BTRFS_MODULES = crc32c
