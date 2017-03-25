PUT_DIRS  ?=
PUT_FILES ?=

# UDEV utilities
PUT_FILES += \
	/sbin/udevd \
	/sbin/udevadm \
	/lib/udev/ata_id \
	/lib/udev/cdrom_id \
	/lib/udev/edd_id \
	/lib/udev/scsi_id

UDEV_FILES_OLD = \
	/lib/udev/vol_id \
	/lib/udev/path_id \
	/lib/udev/usb_id \
	/lib/udev/firmware

PUT_FILES += $(wildcard $(UDEV_FILES_OLD))

# System utilities
PUT_FILES += \
	/sbin/blkid \
	/sbin/chroot \
	/sbin/depmod \
	/sbin/lsmod \
	/sbin/modprobe \
	/sbin/reboot \
	/sbin/poweroff \
	/sbin/halt \
	/bin/sh \
	/bin/cat \
	/bin/cp \
	/bin/chmod \
	/bin/dmesg \
	/bin/ln \
	/bin/ls \
	/bin/mkdir \
	/bin/mknod \
	/bin/mktemp \
	/bin/mount \
	/bin/mountpoint \
	/bin/mv \
	/bin/umount \
	/bin/readlink \
	/bin/rm \
	/bin/rmdir \
	/bin/sleep \
	/usr/bin/logger \
	/usr/bin/setsid

# initrd specific utilities
PUT_DIRS += /lib/initrd $(INITRDDIR) $(DATADIR)
