MULTIPATH_PRELOAD = \
	dm-multipath \
	dm-queue-length \
	dm-round-robin \
	dm-service-time \
	scsi_dh_alua \
	scsi_dh_emc \
	scsi_dh_rdac

MULTIPATH_PATTERN_SET = name:^scsi_dh

MULTIPATH_BIN     = /sbin/multipath /sbin/kpartx /lib/udev/kpartx_id
MULTIPATH_CONF    = /etc/multipath.conf /etc/multipath
MULTIPATH_DATA    = \
	$(wildcard /lib/udev/rules.d/*-kpartx.rules) \
	$(wildcard /lib/udev/rules.d/*-multipath.rules)
