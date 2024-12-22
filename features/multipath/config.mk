# SPDX-License-Identifier: GPL-3.0-or-later
$(call feature-requires,add-modules depmod-image add-udev-rules devmapper)

# Summary: Utilities for devices that use SCSI command sets
# URL: https://sg.danny.cz/sg/sg3_utils.html
MULTIPATH_SG3_PROGS_PATTERNS = \
	*/sg_inq

MULTIPATH_SG3_RULES = \
	*-fc-wwpn-id.rules \
	*-scsi-sg3*.rules \
	*-usb-blacklist.rules

MULTIPATH_SG3_FILES = \
	$(wildcard /lib/udev/fc_wwpn_id)

# Summary: Tools to manage multipath devices with device-mapper
# URL: http://christophe.varoqui.free.fr
MULTIPATH_PRELOAD = \
	dm-multipath \
	dm-queue-length \
	dm-round-robin \
	dm-service-time \
	scsi_dh_alua \
	scsi_dh_emc \
	scsi_dh_rdac

MULTIPATH_PROGS = multipathd multipath kpartx
MULTIPATH_PROGS_PATTERNS = $(MULTIPATH_SG3_PROGS_PATTERNS)

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
	*-kpartx.rules \
	*-multipath.rules \
	$(MULTIPATH_SG3_RULES)

ifeq "$(MULTIPATH_PLUGINS_DIRS)" ""
MULTIPATH_PLUGINS_DIRS = $(foreach d,$(LIB_DIRS),$(wildcard $(d)/multipath))
endif

MULTIPATH_FILES = \
	$(MULTIPATH_SG3_FILES) \
	$(MULTIPATH_PLUGINS_DIRS)
