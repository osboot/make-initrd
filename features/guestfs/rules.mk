DISABLE_FEATURES += plymouth ucode
DISABLE_GUESS += fstab resume root ucode
FEATURES += add-modules cleanup compress btrfs devmapper luks lvm mdadm qemu

MODULES_TRY_ADD += drivers/scsi/ drivers/ata/ fs/
MODULES_PRELOAD += virtio_console

PUT_FEATURE_DIRS  += $(GUESTFS_DATADIR)
PUT_FEATURE_FILES += $(GUESTFS_FILES)
PUT_FEATURE_PROGS += $(GUESTFS_PROGS)
PUT_FEATURE_PROGS_WILDCARD += $(GUESTFS_PROGS_PATTERNS)
