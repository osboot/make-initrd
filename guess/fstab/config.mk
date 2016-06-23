MOUNTPOINTS += $(shell LIBMOUNT_FSTAB=$(FSTAB) findmnt -s -O x-initrd-mount -no TARGET 2>/dev/null ||:)
