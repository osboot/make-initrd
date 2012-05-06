MODULES_LOAD += $(LUKS_MODULES)

$(call require,devmapper,depmod-image)

luks: devmapper
	@echo "Adding LUKS support ..."
	@put-file "$(ROOTDIR)" $(CRYPTSETUP_BIN)
	@put-tree "$(ROOTDIR)" $(LUKS_DATADIR)

depmod-image: luks
