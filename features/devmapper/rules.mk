MODULES_PRELOAD += dm_mod

$(call require,depmod-image)

devmapper: create
	@echo "Adding device-mapper support ..."
	@put-file "$(ROOTDIR)" $(DMSETUP) $(DM_UDEV_RULES)

depmod-image: devmapper
