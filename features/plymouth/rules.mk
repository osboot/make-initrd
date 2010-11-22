plymouth: create
	@echo "Adding plymouth support ..."
	@$(TOOLSDIR)/put-file $(PLYMOUTH_FILES)
	@$(TOOLSDIR)/put-tree $(PLYMOUTH_DATADIR)
	@modlist="`$(PLYMOUTH_MODULES)`"; \
	[ -z "$$modlist" ] || \
	   $(TOOLSDIR)/add-module $$modlist;

pack: plymouth
