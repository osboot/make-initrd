PUT_DIRS  ?=
PUT_FILES ?=

# UDEV utilities
PUT_FILES += \
	/sbin/udevd \
	/sbin/udevadm \
	/lib/udev/ata_id \
	/lib/udev/cdrom_id \
	/lib/udev/edd_id \
	/lib/udev/scsi_id \

PUT_FILES += \
	$(shell test -f /lib/udev/vol_id && echo /lib/udev/vol_id) \
	$(shell test -f /lib/udev/path_id && echo /lib/udev/path_id) \
	$(shell test -f /lib/udev/usb_id && echo /lib/udev/usb_id) \
	$(shell test -f /lib/udev/firmware && echo /lib/udev/firmware)

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
	/bin/ash \
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
	/bin/mv \
	/bin/umount \
	/bin/readlink \
	/bin/rm \
	/bin/rmdir \
	/bin/sleep \
	/usr/bin/logger

# initrd specific utilities
PUT_DIRS += /lib/initrd
