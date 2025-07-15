# SPDX-License-Identifier: GPL-3.0-or-later

$(call assgin-shell-once,ROOTFS_DIRS,$(FEATURESDIR)/rootfs/bin/create-fstab dirs)

PUT_FEATURE_DIR += $(ROOTFS_DIRS)

$(call set-sysconfig,init,QUIET,$(RUNTIME_QUIET))
$(call set-sysconfig,init,LOGLEVEL,$(RUNTIME_DMESG_LOGLEVEL))
