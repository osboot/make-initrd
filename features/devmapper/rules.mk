$(call require,depmod-image)
devmapper: create
	@echo "Adding device-mapper support ..."
	@$(PUT_FILE) $(DMSETUP) $(DM_UDEV_RULES)
	@$(LOAD_MODULE) --pre-udev dm_mod

depmod-image: devmapper
