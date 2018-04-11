$(call require,syslog)

dropbear:
	@$(MSG) "Adding dropbear server ..."
	@put-tree "$(ROOTDIR)" $(DROPBEAR_DATADIR)
	@put-file "$(ROOTDIR)" $(DROPBEAR_FILES)

pack: dropbear
