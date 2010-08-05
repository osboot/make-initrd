MULTIPATH_PRELOAD	= dm-multipath
MULTIPATH_MODULES	= scsi_dh scsi_dh_alua scsi_dh_emc \
			  scsi_dh_hp_sw scsi_dh_rda

MULTIPATH_BIN		= /sbin/multipath /sbin/kpartx \
			  /lib/udev/kpartx_id

MULTIPATH_CONF		= /etc/multipath.conf

MULTIPATH_DATA          = /lib/udev/rules.d/40-kpartx.rules \
			  /lib/udev/rules.d/40-multipath.rules
