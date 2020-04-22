fsck: create
	@$(MSG) "Adding fsck support ..."
	@put-tree "$(ROOTDIR)" $(FSCK_DATADIR)
	@put-file "$(ROOTDIR)" $(FSCK_FILES)

pack: fsck
