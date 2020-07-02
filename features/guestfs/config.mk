GUESTFS_DATADIR = \
	$(FEATURESDIR)/guestfs/data

GUESTFS_PROGS_PATTERNS = \
	*/guestfsd \
	*/parted \
	*/wipefs \
	*/findfs \
	*/zerofree \
	*/sparsify \
	*/rsync \
	*/xfs_* \
	*/mount* \
	*/fsck* \
	*/mkfs* \
	*/*resize* \
	*disk \
	*/hivex*

GUESTFS_FILES = \
	$(wildcard /usr/share/augeas/*) \
	$(wildcard /usr/lib*/gconv)
