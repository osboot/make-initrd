GUESTFS_DATADIR = \
	$(FEATURESDIR)/guestfs/data

GUESTFS_PROGS = guestfsd parted wipefs zerofree sparsify rsync xfs_repair

GUESTFS_FILES = \
	$(wildcard /sbin/mount*) \
	$(wildcard /sbin/fsck*) \
	$(wildcard /sbin/mkfs*) \
	$(wildcard /sbin/*resize*) \
	$(wildcard /sbin/*disk) \
	$(wildcard /sbin/mount*) \
	$(wildcard /usr/sbin/xfs_*) \
	$(wildcard /usr/bin/hivex*) \
	$(wildcard /usr/share/augeas/*) \
	$(wildcard /usr/lib*/gconv)
