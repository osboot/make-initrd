#
# Search for options:
#   comment="x-initrd-mount"
#   x-initrd-mount
#
FSTAB_MOUNTPOINTS := $(shell LIBMOUNT_FSTAB=$(FSTAB) findmnt -s -O x-initrd-mount -no TARGET 2>/dev/null ||:)

MOUNTPOINTS += $(FSTAB_MOUNTPOINTS)

GUESSv1_ROOTFS_SOURCE := $(current-feature-name) root
GUESSv1_ROOTFS_RULE = EMPTYVAR{MOUNTPOINTS}==false EXISTS{$(SYSFS_PATH)} ACTION{RUN}=$(FEATURESDIR)/rootfs/bin/guess
