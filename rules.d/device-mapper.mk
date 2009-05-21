ifndef _DEVICE_MAPPER_MK
_DEVICE_MAPPER_MK = 1

device-mapper: create
	@echo "Adding device-mapper support ..."
	@$(PUT_FILE) $(DMSETUP) $(UDEV_DM_EXPORT)
	@$(LOAD_MODULE) dm_mod

depmod-image: device-mapper

include $(RULESDIR)/depmod-image.mk
endif
