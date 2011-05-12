MODULES_PRELOAD += $(BTRFS_PRELOAD)

btrfs:
	@echo "Adding btrfs support ..."
	@$(TOOLSDIR)/put-file $(BTRFS_FILES)
	@$(TOOLSDIR)/put-tree $(BTRFS_DATADIR)

pack: btrfs
