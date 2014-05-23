$(call require,syslog)

dropbear:
	@echo "Adding dropbear server ..."
	@put-tree "$(ROOTDIR)" $(DROPBEAR_DATADIR)
	@put-file "$(ROOTDIR)" $(DROPBEAR_FILES)

pack: dropbear
