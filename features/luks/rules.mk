MODULES_LOAD += $(LUKS_MODULES)

$(call require,devmapper,depmod-image)

luks: devmapper
	@echo "Adding LUKS support ..."
	@$(TOOLSDIR)/put-file $(CRYPTSETUP_BIN)
	@$(TOOLSDIR)/put-tree $(LUKS_DATADIR)

depmod-image: luks
