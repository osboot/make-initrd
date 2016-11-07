MULTIPATH_PRELOAD	= scsi_dh_alua scsi_dh_emc scsi_dh_rdac \
			  dm-multipath dm-service-time dm-queue-length dm-round-robin

MULTIPATH_BIN		= /sbin/multipathd /sbin/multipath /$(LIBNAME)/multipath \
			  /sbin/kpartx /lib/udev/kpartx_id

MULTIPATH_CONF		= /etc/multipath.conf /etc/multipath

MULTIPATH_DATA          = /lib/udev/rules.d/11-dm-mpath.rules \
			  /lib/udev/rules.d/56-multipath.rules \
			  /lib/udev/rules.d/66-kpartx.rules

MULTIPATH_DATADIR	= $(FEATURESDIR)/multipath/data
