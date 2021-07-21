#
# Search for options:
#   comment="x-initrd-mount"
#   x-initrd-mount
#
FSTAB_MOUNTPOINTS := $(shell LIBMOUNT_FSTAB=$(FSTAB) findmnt -s -O x-initrd-mount -no TARGET 2>/dev/null ||:)

MOUNTPOINTS += $(FSTAB_MOUNTPOINTS)

GUESSv1_ROOTFS_RULE = EXISTS{$(SYSFS_PATH)} ACTION{RUN}=$(FEATURESDIR)/rootfs/bin/guess
