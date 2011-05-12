systemd:
	@echo "Adding systemd support ..."
	@$(TOOLSDIR)/put-tree $(SYSTEMD_DATADIR)

pack: systemd
