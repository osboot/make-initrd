MODULES_PRELOAD += dm_mod
MODULES_ADD     += dm_snapshot

$(call require,depmod-image)

devmapper: create
	@echo "Adding device-mapper support ..."
	@put-file "$(ROOTDIR)" $(DMSETUP) $(DM_UDEV_RULES)
	@put-tree "$(ROOTDIR)" $(DM_DATADIR)

depmod-image: devmapper
