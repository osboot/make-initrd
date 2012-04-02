MODULES_ADD += drivers/md/

raid:
	@echo "Adding RAID support ..."
	@$(TOOLSDIR)/put-tree $(RAID_DATADIR)

pack: raid
