MODULES_PRELOAD += $(BTRFS_PRELOAD)
MODULES_ADD     += $(BTRFS_MODULES)

btrfs:
	@echo "Adding btrfs support ..."
	@$(TOOLSDIR)/put-file $(BTRFS_FILES)
	@$(TOOLSDIR)/put-tree $(BTRFS_DATADIR)

pack: btrfs
