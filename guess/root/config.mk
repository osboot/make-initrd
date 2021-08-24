#
# Search for options:
#   comment="x-initrd-mount"
#   x-initrd-mount
#
GUESS_MOUNTPOINTS += / $(shell LIBMOUNT_FSTAB=$(FSTAB) findmnt -s -O x-initrd-mount -no TARGET 2>/dev/null ||:)
