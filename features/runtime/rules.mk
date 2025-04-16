# SPDX-License-Identifier: GPL-3.0-or-later

PHONY += create

PUT_FEATURE_PROGS += \
	blkid cat chmod chroot cp depmod dmesg findmnt grep kill killall5 ln \
	logger ls lsmod mkdir mknod mktemp modprobe mount mountpoint mv \
	readlink rm rmdir setsid sh sleep start-stop-daemon switch_root \
	touch umount which

PUT_FEATURE_PROGS_WILDCARD = \
	*/shell-*

PUT_FEATURE_FILES += $(UDEVD) $(UDEVADM)

PUT_FEATURE_DIRS += $(FEATURESDIR)/runtime/data

create: depmod-host
	@$(VMSG) "Creating initrd image ..."
	@mkdir -m 755 -p $(verbose) -- $(ROOTDIR)
	@$(TOOLSDIR)/create-initrd
