# SPDX-License-Identifier: GPL-3.0-or-later
$(call feature-requires,depmod-image add-udev-rules)

PIPELINE_DATADIR = $(FEATURESDIR)/pipeline/data

PIPELINE_RULES = \
	*-cdrom_id.rules

PIPELINE_MODULES = fs-iso9660 fs-squashfs fs-overlay devname:loop-control
PIPELINE_PROGS = ip ping wget
