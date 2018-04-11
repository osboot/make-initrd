MODULES_LOAD += $(LUKS_MODULES)

$(call require,devmapper,depmod-image)

luks: devmapper
	@$(MSG) "Adding LUKS support ..."
	@put-file "$(ROOTDIR)" $(CRYPTSETUP_BIN)
	@put-tree "$(ROOTDIR)" $(LUKS_DATADIR)
	@if [ -n "$(LUKS_USE_GPG)" ]; then \
	    if [ ! -x "$(GPG_PROGRAM)" ]; then \
	        echo "File not found: $(GPG_PROGRAM)" >&2; \
	        exit 1; \
	    fi; \
	    put-file "$(ROOTDIR)" "$(GPG_PROGRAM)"; \
	fi

depmod-image: luks
