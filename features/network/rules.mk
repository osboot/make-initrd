MODULES_ADD += $(NETWORK_MODULES)

$(call require,depmod-image)

network:
	@$(MSG) "Adding network support ..."
	@put-file "$(ROOTDIR)" $(NETWORK_FILES)
	@put-tree "$(ROOTDIR)" $(NETWORK_DATADIR)

depmod-image: network
