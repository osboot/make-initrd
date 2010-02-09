$(call require,devmapper,depmod-image)

luks: devmapper
	@echo "Adding LUKS support ..."
	@$(TOOLSDIR)/put-file $(CRYPTSETUP_BIN)
	@$(TOOLSDIR)/put-tree $(LUKS_DATADIR)
	@$(TOOLSDIR)/load-module dm-crypt $(LUKS_CIPHERS) $(LUKS_BLOCKCIPHERS) $(LUKS_HASHES)

depmod-image: luks
