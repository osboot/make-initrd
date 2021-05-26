$(call feature-requires,depmod-image devmapper)

MULTIPATH_PRELOAD = \
	dm-multipath \
	dm-queue-length \
	dm-round-robin \
	dm-service-time \
	scsi_dh_alua \
	scsi_dh_emc \
	scsi_dh_rdac

MULTIPATH_PROGS   = multipathd multipath kpartx
MULTIPATH_CONF    = \
	$(SYSCONFDIR)/multipath.conf \
	$(SYSCONFDIR)/multipath
MULTIPATH_DATADIR = \
	$(FEATURESDIR)/multipath/data
MULTIPATH_RULES   = \
	*-kpartx.rules \
	*-multipath.rules
