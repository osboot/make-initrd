# SPDX-License-Identifier: GPL-3.0-or-later
$(call feature-requires,add-modules depmod-image devmapper luks lvm mdadm modules-filesystem system-glibc network)

KICKSTART_PROGS  = sfdisk wipefs blkid findmnt mkswap mount mountpoint chroot \
		   rsync wget tar unzip cpio env sha256sum eject halt reboot \
		   btrfs mkfs.btrfs mkfs.ext4 numfmt e2fsck resize2fs \
		   xfs_growfs resize.f2fs

KICKSTART_PROGS_PATTERNS = \
			   */lz4 \
			   */zstd \
			   */zpool \
			   */zfs \
			   */mkfs.ext* \
			   */mkfs.xfs \
			   */mkfs.fat \
			   */mkfs.reiserfs \
			   */mkfs.vfat \
			   */mkfs.f2fs \
			   #

KICKSTART_DATADIR = $(FEATURESDIR)/kickstart/data

KICKSTART_CONFIGS ?=
