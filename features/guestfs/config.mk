DISABLE_FEATURES += plymouth ucode
DISABLE_GUESS += fstab resume root ucode
FEATURES += add-modules cleanup compress btrfs devmapper luks lvm mdadm qemu


GUESTFS_DATADIR = \
	$(FEATURESDIR)/guestfs/data


GUESTFS_FILES = \
	/usr/sbin/guestfsd \
	/usr/sbin/parted \
	/sbin/wipefs \
	/usr/sbin/zerofree \
	/usr/sbin/sparsify \
	/usr/bin/rsync \
	$(wildcard /sbin/mount*) \
	$(wildcard /sbin/fsck*) \
	$(wildcard /sbin/mkfs*) \
	$(wildcard /sbin/*resize*) \
	$(wildcard /sbin/*disk) \
	$(wildcard /sbin/mount*) \
	$(wildcard /usr/sbin/xfs_*) \
	/sbin/xfs_repair \
	$(wildcard /usr/bin/hivex*) \
	$(wildcard /usr/share/augeas/*) \
	$(wildcard /usr/lib*/gconv)
