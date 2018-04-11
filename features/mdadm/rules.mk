mdadm:
	@$(MSG) "Adding mdadm support ..."
	@put-file "$(ROOTDIR)" $(MDADM_FILES)
	@put-tree "$(ROOTDIR)" $(MDADM_DATADIR)

pack: mdadm
