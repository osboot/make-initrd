DISABLE_FEATURES += plymouth ucode
DISABLE_GUESS += fstab resume root ucode
FEATURES += add-modules cleanup compress btrfs devmapper luks lvm mdadm qemu

MODULES_ADD += drivers/scsi/ drivers/ata/ fs/
MODULES_PRELOAD += virtio_console

guestfs: create
	@$(MSG) "Adding guestfs support ..."
	@put-tree "$(ROOTDIR)" $(GUESTFS_DATADIR)
	@put-file "$(ROOTDIR)" $(GUESTFS_FILES)

pack: guestfs
