raid:
	@$(MSG) "Adding RAID support ..."
	@put-tree "$(ROOTDIR)" $(RAID_DATADIR)

pack: raid
