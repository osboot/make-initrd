$(call feature-requires,depmod-image add-udev-rules)

DM_PROGS	?= dmsetup
DM_UDEV_RULES	?= *-dm.rules \
		   *-dm-lvm.rules \
		   *-dm-disk.rules \
		   *-dm-notify.rules \
		   $(wildcard $(FEATURESDIR)/devmapper/rules.d/*.rules)
