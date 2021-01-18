$(call feature-requires,add-modules qemu mdadm lvm luks devmapper btrfs compress cleanup)
$(call feature-disables,plymouth ucode)

GUESTFS_DATADIR = \
	$(FEATURESDIR)/guestfs/data

GUESTFS_FILE_DATABASE = \
	$(wildcard /usr/share/file/*) \
	$(wildcard /usr/share/magic/*) \
	$(wildcard /etc/magic)

GUESTFS_PROGS_PATTERNS = \
	*/guestfsd \
	*/parted \
	*/wipefs \
	*/findfs \
	*/file \
	*/zerofree \
	*/sparsify \
	*/rsync \
	*/xfs_* \
	*/mount* \
	*/fsck* \
	*/mkfs* \
	*/mke2fs \
	*/*label \
	*/*resize* \
	*disk \
	*/hivex*

GUESTFS_FILES = \
	$(GUESTFS_FILE_DATABASE) \
	$(wildcard /usr/share/augeas/*) \
	$(wildcard /usr/lib*/gconv)
