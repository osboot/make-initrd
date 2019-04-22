PUT_DIRS  ?=
PUT_FILES ?=
PUT_PROGS ?=

# UDEV utilities
PUT_FILES += \
	/sbin/udevd \
	/sbin/udevadm \
	/lib/udev/ata_id \
	/lib/udev/cdrom_id \
	/lib/udev/scsi_id

UDEV_FILES_OLD = \
	/lib/udev/edd_id \
	/lib/udev/vol_id \
	/lib/udev/path_id \
	/lib/udev/usb_id \
	/lib/udev/firmware

PUT_FILES += $(wildcard $(UDEV_FILES_OLD))

# System utilities
PUT_FILES += \
	/bin/cat \
	/bin/chmod \
	/bin/cp \
	/bin/dmesg \
	/bin/findmnt \
	/bin/kill \
	/bin/ln \
	/bin/ls \
	/bin/mkdir \
	/bin/mknod \
	/bin/mktemp \
	/bin/mount \
	/bin/mountpoint \
	/bin/mv \
	/bin/readlink \
	/bin/rm \
	/bin/rmdir \
	/bin/runas \
	/bin/sh \
	/bin/sleep \
	/bin/touch \
	/bin/umount \
	/sbin/blkid \
	/sbin/chroot \
	/sbin/depmod \
	/sbin/killall5 \
	/sbin/lsmod \
	/sbin/modprobe \
	/sbin/start-stop-daemon \
	/sbin/switch_root \
	/usr/bin/logger \
	/usr/bin/setsid \
	/usr/bin/which \
	/usr/share/terminfo/l \
	/var/resolv

# Add libshell
PUT_FILES += $(wildcard /bin/shell-*)

# initrd specific utilities
PUT_DIRS += /lib/initrd $(DATADIR)
