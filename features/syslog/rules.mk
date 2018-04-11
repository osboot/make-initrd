syslog:
	@$(MSG) "Adding syslog support ..."
	@put-tree "$(ROOTDIR)" $(SYSLOG_DATADIR)
	@put-file "$(ROOTDIR)" $(SYSLOG_FILES)

pack: syslog
