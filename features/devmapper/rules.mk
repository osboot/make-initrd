$(call require,depmod-image)
devmapper: create
	@echo "Adding device-mapper support ..."
	@$(TOOLSDIR)/put-file $(DMSETUP) $(DM_UDEV_RULES)
	@$(TOOLSDIR)/load-module --pre-udev dm_mod

depmod-image: devmapper
