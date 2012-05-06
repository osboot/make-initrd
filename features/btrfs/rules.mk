MODULES_PRELOAD += $(BTRFS_PRELOAD)
MODULES_ADD     += $(BTRFS_MODULES)

btrfs:
	@echo "Adding btrfs support ..."
	@put-file "$(ROOTDIR)" $(BTRFS_FILES)
	@put-tree "$(ROOTDIR)" $(BTRFS_DATADIR)

pack: btrfs
