MODULES_PRELOAD += dm_mod
MODULES_ADD     += dm_snapshot

PUT_FEATURE_DIRS  += $(DM_DATADIR)
PUT_FEATURE_FILES += $(DMSETUP) $(DM_UDEV_RULES)

$(call require,depmod-image)
