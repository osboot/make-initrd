$(call feature-requires,devmapper luks lvm mdadm modules-filesystem system-glibc network)

KICKSTART_PROGS  = sfdisk wipefs blkid findmnt mkswap mount mountpoint \
		   rsync wget tar unzip cpio env sha256sum eject halt reboot

KICKSTART_PROGS_PATTERNS = \
			   */mkfs.btrfs \
			   */mkfs.ext* \
			   */mkfs.xfs \
			   */mkfs.fat \
			   */mkfs.reiserfs

KICKSTART_FILES = \
		  /lib/udev/rules.d/63-md-raid-arrays.rules \
		  /lib/udev/rules.d/64-md-raid-assembly.rules

KICKSTART_DATADIR = $(FEATURESDIR)/kickstart/data

KICKSTART_CONFIGS ?=
