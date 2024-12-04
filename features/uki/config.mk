# SPDX-License-Identifier: GPL-3.0-or-later

$(call feature-requires,\
	add-modules add-udev-rules \
	btrfs mdadm lvm luks devmapper \
	modules-blockdev \
	modules-crypto-user-api \
	modules-filesystem \
	modules-multiple-devices \
	modules-network \
	modules-nfs)

$(call feature-disables,compress)

ifeq "$(UKI)" "1"
$(info "XXX!")
endif
