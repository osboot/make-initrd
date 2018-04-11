MODULES_PRELOAD	+= $(MULTIPATH_PRELOAD)
MODULES_ADD	+= $(MULTIPATH_MODULES)

$(call require,devmapper)

multipath: devmapper
	@$(MSG) "Adding multipath support ..."
	@put-file "$(ROOTDIR)" $(MULTIPATH_BIN)
	@put-file "$(ROOTDIR)" $(MULTIPATH_CONF)
	@put-file "$(ROOTDIR)" $(MULTIPATH_DATA)

pack: multipath
