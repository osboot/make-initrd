# SPDX-License-Identifier: GPL-3.0-or-later
$(call feature-requires,depmod-image add-udev-rules)

BTRFS_UDEV_RULES := $(wildcard $(FEATURESDIR)/btrfs/rules.d/*.rules)
BTRFS_PROGS   = btrfs
BTRFS_PRELOAD = btrfs
BTRFS_MODULES = crc32c
