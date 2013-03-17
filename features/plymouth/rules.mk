MODULES_ADD += $(shell $(PLYMOUTH_MODULES))

plymouth: create
	@echo "Adding plymouth support ..."
	@put-file "$(ROOTDIR)" $(PLYMOUTH_FILES)
	@put-tree "$(ROOTDIR)" $(PLYMOUTH_DATADIR)

pack: plymouth
depmod-image: plymouth
