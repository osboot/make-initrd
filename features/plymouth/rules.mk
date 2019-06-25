MODULES_ADD += $(shell $(PLYMOUTH_MODULES))
MODULES_TRY_ADD += drivers/char/agp

plymouth: create
	@$(MSG) "Adding plymouth support ..."
	@put-file "$(ROOTDIR)" $(PLYMOUTH_FILES) $(PLYMOUTH_RULES)
	@put-tree "$(ROOTDIR)" $(PLYMOUTH_DATADIR)

pack: plymouth
depmod-image: plymouth
