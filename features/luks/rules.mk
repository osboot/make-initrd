$(call require,device-mapper,depmod-image)

luks: device-mapper
	@echo "Adding LUKS support ..."
	@$(PUT_FILE) $(CRYPTSETUP_BIN)
	@$(PUT_TREE) $(LUKS_DATADIR)
	@$(LOAD_MODULE) dm-crypt $(LUKS_CIPHERS) $(LUKS_BLOCKCIPHERS) $(LUKS_HASHES)

depmod-image: luks
