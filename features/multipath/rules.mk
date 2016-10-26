MODULES_PRELOAD	+= $(MULTIPATH_PRELOAD)

$(call require,devmapper)

multipath: devmapper
	@echo "Adding multipath support ..."
	@put-file "$(ROOTDIR)" $(MULTIPATH_BIN)
	@put-file "$(ROOTDIR)" $(MULTIPATH_CONF)
	@put-file "$(ROOTDIR)" $(MULTIPATH_DATA)
	@put-tree "$(ROOTDIR)" $(MULTIPATH_DATADIR)

pack: multipath
