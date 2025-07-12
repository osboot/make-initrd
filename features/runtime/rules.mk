# SPDX-License-Identifier: GPL-3.0-or-later

PHONY += create

PUT_FEATURE_PROGS += \
	blkid cat chmod chroot cp depmod dmesg findmnt grep kill killall5 ln \
	logger ls lsmod mkdir mknod mktemp modprobe mount mountpoint mv \
	readlink rm rmdir setsid sh sleep start-stop-daemon \
	touch umount which

PUT_FEATURE_PROGS_WILDCARD = \
	*/shell-*

PUT_FEATURE_FILES += $(UDEVD) $(UDEVADM)

PUT_FEATURE_DIRS += $(FEATURESDIR)/runtime/data

create:
	@$(VMSG) "Creating rootfs ..."
	@mkdir -m 755 -p $(verbose3) -- $(ROOTDIR)
	@$(TOOLSDIR)/create-rootfs
