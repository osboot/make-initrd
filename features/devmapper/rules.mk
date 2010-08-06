MODULES_PRELOAD += dm_mod

$(call require,depmod-image)

devmapper: create
	@echo "Adding device-mapper support ..."
	@$(TOOLSDIR)/put-file $(DMSETUP) $(DM_UDEV_RULES)

depmod-image: devmapper
