$(call feature-requires,depmod-image add-udev-rules)

DM_PROGS	?= dmsetup
DM_UDEV_RULES	?= *-dm.rules \
		   *-dm-lvm.rules \
		   *-dm-disk.rules \
		   *-dm-notify.rules
DM_DATADIR	?= $(FEATURESDIR)/devmapper/data
