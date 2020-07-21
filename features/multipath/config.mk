$(call feature-requires,depmod-image devmapper)

MULTIPATH_PRELOAD = \
	dm-multipath \
	dm-queue-length \
	dm-round-robin \
	dm-service-time \
	scsi_dh_alua \
	scsi_dh_emc \
	scsi_dh_rdac

MULTIPATH_PROGS   = multipath kpartx
MULTIPATH_CONF    = \
	$(SYSCONFDIR)/multipath.conf \
	$(SYSCONFDIR)/multipath
MULTIPATH_DATA    = \
	/lib/udev/kpartx_id \
	$(wildcard /lib/udev/rules.d/*-kpartx.rules) \
	$(wildcard /lib/udev/rules.d/*-multipath.rules)
