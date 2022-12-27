MODULES_TRY_ADD += drivers/scsi/ drivers/ata/ fs/
MODULES_TRY_ADD += dm-raid raid0 raid1 raid456 raid10
MODULES_PRELOAD += virtio_console

PUT_UDEV_RULES    += $(GUESTFS_UDEV_RULES)
PUT_FEATURE_DIRS  += $(GUESTFS_DATADIR)
PUT_FEATURE_FILES += $(GUESTFS_FILES)
PUT_FEATURE_PROGS += $(GUESTFS_PROGS)
PUT_FEATURE_PROGS_WILDCARD += $(GUESTFS_PROGS_PATTERNS)

# libugestfs doesn't work with busybox's swapon, so replace it
.PHONY: guestfs

guestfs: create
	@put-file -f "$(ROOTDIR)" /sbin/swapon
	@put-file -f "$(ROOTDIR)" /sbin/swapoff

pack: guestfs
