$(call require,depmod-image)
device-mapper: create
	@echo "Adding device-mapper support ..."
	@$(PUT_FILE) $(DMSETUP) $(UDEV_DM_EXPORT)
	@$(LOAD_MODULE) --pre-udev dm_mod

depmod-image: device-mapper
