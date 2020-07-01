MODULES_PRELOAD += dm_mod
MODULES_ADD     += dm_snapshot

PUT_FEATURE_DIRS  += $(DM_DATADIR)
PUT_FEATURE_FILES += $(DM_UDEV_RULES)
PUT_FEATURE_PROGS += $(DM_PROGS)

$(call require,depmod-image)
