MODULES_PRELOAD	+= $(MULTIPATH_PRELOAD)

MODULES_PATTERN_SETS += MULTIPATH_PATTERN_SET

$(call require,devmapper)

multipath: devmapper
	@$(MSG) "Adding multipath support ..."
	@put-file "$(ROOTDIR)" $(MULTIPATH_BIN)
	@put-file "$(ROOTDIR)" $(MULTIPATH_CONF)
	@put-file "$(ROOTDIR)" $(MULTIPATH_DATA)

pack: multipath
