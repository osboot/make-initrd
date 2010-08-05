MODULES_PRELOAD	+= $(MULTIPATH_PRELOAD)
MODULES_ADD	+= $(MULTIPATH_MODULES)

$(call require,devmapper)

multipath: devmapper
	@echo "Adding multipath support ..."
	@$(TOOLSDIR)/put-file $(MULTIPATH_BIN)
	@$(TOOLSDIR)/put-file $(MULTIPATH_CONF)
	@$(TOOLSDIR)/put-file $(MULTIPATH_DATA)

pack: multipath
