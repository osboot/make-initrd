$(call feature-requires,add-modules add-udev-rules qemu mdadm lvm luks devmapper btrfs compress cleanup)
$(call feature-disables,plymouth ucode)

GUESTFS_DATADIR = \
	$(FEATURESDIR)/guestfs/data

GUESTFS_UDEV_RULES := \
	$(wildcard $(FEATURESDIR)/guestfs/rules.d/*.rules) \
	*-md-raid-arrays.rules \
	*-md-raid-assembly.rules

GUESTFS_FILE_DATABASE = \
	$(wildcard /usr/share/file/*) \
	$(wildcard /usr/share/magic/*) \
	$(wildcard /etc/magic)

GUESTFS_PROGS = cpio blockdev btrfs-image btrfsck btrfstune \
	mklost+found mkswap tune2fs base64 chattr cksum lsattr \
	md5sum sha1sum sha224sum sha256sum sha384sum sha512sum \
	strings uuidgen zcat zgrep

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
