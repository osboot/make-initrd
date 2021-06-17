$(call feature-requires,depmod-image add-udev-rules)

PIPELINE_DATADIR = $(FEATURESDIR)/pipeline/data

PIPELINE_RULES = \
	*-cdrom_id.rules

PIPELINE_MODULES = isofs squashfs overlay
