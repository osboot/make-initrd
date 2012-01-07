syslog:
	@echo "Adding syslog support ..."
	@$(TOOLSDIR)/put-tree $(SYSLOG_DATADIR)
	@$(TOOLSDIR)/put-file $(SYSLOG_FILES)

pack: syslog
