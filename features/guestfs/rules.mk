MODULES_ADD += drivers/scsi/ drivers/ata/ fs/
MODULES_PRELOAD += virtio_console

guestfs: create
	@$(MSG) "Adding guestfs support ..."
	@put-tree "$(ROOTDIR)" $(GUESTFS_DATADIR)
	@put-file "$(ROOTDIR)" $(GUESTFS_FILES)

pack: guestfs
