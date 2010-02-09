raid:
	@echo "Adding RAID support ..."
	@$(TOOLSDIR)/put-tree $(RAID_DATADIR)

pack: raid
