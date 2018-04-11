telnetd:
	@$(MSG) "Adding telnet server ..."
	@put-tree "$(ROOTDIR)" $(TELNET_DATADIR)

pack: telnetd
