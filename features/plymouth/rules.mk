plymouth: create
	@echo "Adding plymouth support ..."
	@put-file "$(ROOTDIR)" $(PLYMOUTH_FILES)
	@put-tree "$(ROOTDIR)" $(PLYMOUTH_DATADIR)
	@modlist="`$(PLYMOUTH_MODULES)`"; \
	[ -z "$$modlist" ] || \
	   $(TOOLSDIR)/add-module $$modlist;

pack: plymouth
