# SPDX-License-Identifier: GPL-3.0-or-later
KICKSTART_EXTRA_FEATURES ?= devmapper luks lvm mdadm modules-filesystem network

$(call feature-requires,add-modules depmod-image system-glibc $(KICKSTART_EXTRA_FEATURES))

KICKSTART_PROGS ?= \
	sfdisk wipefs blkid findmnt mount mountpoint chroot \
	wget tar env sha256sum numfmt

KICKSTART_PROGS_PATTERNS ?= \
	*/rsync \
	*/unzip \
	*/cpio \
	*/eject \
	*/halt \
	*/reboot \
	*/mkswap \
	*/lz4 \
	*/zstd \
	*/zpool \
	*/zfs \
	*/btrfs \
	*/mkfs.btrfs \
	*/mkfs.ext* \
	*/e2fsck \
	*/resize2fs \
	*/mkfs.xfs \
	*/xfs_growfs \
	*/mkfs.fat \
	*/mkfs.reiserfs \
	*/mkfs.vfat \
	*/mkfs.f2fs \
	*/fsck.f2fs \
	*/resize.f2fs \
	#

KICKSTART_DATADIR = $(FEATURESDIR)/kickstart/data

KICKSTART_CONFIGS ?=
