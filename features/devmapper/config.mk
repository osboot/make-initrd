# SPDX-License-Identifier: GPL-3.0-or-later
$(call feature-requires,add-modules depmod-image add-udev-rules)

DM_PROGS	?= dmsetup
DM_UDEV_RULES	?= *-dm.rules \
		   *-dm-lvm.rules \
		   *-dm-disk.rules \
		   *-dm-notify.rules \
		   $(wildcard $(FEATURESDIR)/devmapper/rules.d/*.rules)
