$(call feature-requires,devmapper luks lvm mdadm modules-filesystem system-glibc network)

KICKSTART_PROGS  = sfdisk wipefs blkid findmnt mkswap mount mountpoint chroot \
		   rsync wget tar unzip cpio env sha256sum eject halt reboot \
		   btrfs mkfs.btrfs mkfs.ext4

KICKSTART_PROGS_PATTERNS = \
			   */lz4 \
			   */zstd \
			   */mkfs.ext* \
			   */mkfs.xfs \
			   */mkfs.fat \
			   */mkfs.reiserfs \
			   */mkfs.vfat \
			   #

KICKSTART_DATADIR = $(FEATURESDIR)/kickstart/data

KICKSTART_CONFIGS ?=
