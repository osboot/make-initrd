MULTIPATH_PRELOAD	= scsi_dh_alua scsi_dh_emc scsi_dh_rdac dm-multipath

MULTIPATH_BIN		= /sbin/multipathd /sbin/multipath \
			  /$(LIBNAME)/multipath /$(LIBNAME)/libmultipath.so.0

MULTIPATH_CONF		= /etc/multipath.conf /etc/multipath

MULTIPATH_DATA          = /lib/udev/rules.d/11-dm-mpath.rules \
			  /lib/udev/rules.d/56-multipath.rules

MULTIPATH_DATADIR	= $(FEATURESDIR)/multipath/data
