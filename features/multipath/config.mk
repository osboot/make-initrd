MULTIPATH_PRELOAD	= dm-multipath
MULTIPATH_MODULES	= scsi_dh scsi_dh_alua scsi_dh_emc \
			  scsi_dh_hp_sw scsi_dh_rdac

MULTIPATH_BIN		= /sbin/multipath /sbin/kpartx \
			  /lib/udev/kpartx_id

MULTIPATH_CONF		= /etc/multipath.conf

MULTIPATH_DATA          = /lib/udev/rules.d/11-dm-mpath.rules \
			  /lib/udev/rules.d/56-multipath.rules \
			  /lib/udev/rules.d/66-kpartx.rules
