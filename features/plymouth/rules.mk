MODULES_ADD += $(PLYMOUTH_MODULES)

plymouth: create
	@echo "Adding plymouth support ..."
	@$(TOOLSDIR)/put-file $(PLYMOUTH_FILES)
	@$(TOOLSDIR)/put-tree $(PLYMOUTH_DATADIR)

pack: plymouth
