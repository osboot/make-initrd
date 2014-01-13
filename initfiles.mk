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
	/bin/mountpoint \
	/bin/mv \
	/bin/umount \
	/bin/readlink \
	/bin/rm \
	/bin/rmdir \
	/bin/sleep \
	/usr/bin/logger

# initrd specific utilities
PUT_DIRS += /lib/initrd $(DATADIR)

# Add libshell
PUT_FILES += $(wildcard /bin/shell-*)

PUT_FILES += \
	/bin/bash \
	/bin/dmesg \
	/bin/touch \
	/bin/ps \
	/bin/sleep \
	/bin/getopt \
	/usr/bin/flock \
	/usr/bin/ip \
	/usr/bin/which \
	/sbin/limited \
	/sbin/chkconfig \
	/sbin/initlog \
	/sbin/runlevel \
	/sbin/service \
	/sbin/start-stop-daemon \
	/etc/initlog.conf \
	/etc/sysconfig/limits \
	/etc/sysconfig/klogd \
	/etc/sysconfig/syslogd \
	/var/lib/klogd \
	/var/resolv
