# SPDX-License-Identifier: GPL-3.0-or-later

$(call set-sysconfig,init,QUIET,$(RUNTIME_QUIET))
$(call set-sysconfig,init,LOGLEVEL,$(RUNTIME_DMESG_LOGLEVEL))

rootfs-fstab: create
	@$(VMSG) "Generating fstab..."
	@$(FEATURESDIR)/rootfs/bin/create-fstab

pack: rootfs-fstab
