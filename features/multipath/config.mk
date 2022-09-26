$(call feature-requires,depmod-image add-udev-rules devmapper)

MULTIPATH_PRELOAD = \
	dm-multipath \
	dm-queue-length \
	dm-round-robin \
	dm-service-time \
	scsi_dh_alua \
	scsi_dh_emc \
	scsi_dh_rdac

MULTIPATH_PROGS   = multipathd multipath kpartx sg_inq
MULTIPATH_CONF    = \
	$(SYSCONFDIR)/multipath.conf \
	$(SYSCONFDIR)/multipath \
	$(SYSCONFDIR)/multipath/bindings \
	$(SYSCONFDIR)/multipath/wwids
MULTIPATH_DATADIR = \
	$(FEATURESDIR)/multipath/data
MULTIPATH_RULES   = \
	*-dm-mpath.rules \
	*-dm-parts.rules \
	*-fc-wwpn-id.rules \
	*-kpartx.rules \
	*-multipath.rules \
	*-scsi-sg3*.rules \
	*-usb-blacklist.rules
MULTIPATH_FILES = /lib/udev/fc_wwpn_id
