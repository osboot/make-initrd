RAID_DATADIR = $(FEATURESDIR)/raid/data

# Raid rules have been removed from udev-151 and later.
# They are necessary in case of use md_run.
RAID_UDEV_RULES := $(shell test "$(UDEV_VERSION)" -gt "150" && echo "TRUE")

ifeq "$(RAID_UDEV_RULES)" 'TRUE'
RAID_DATADIR += $(FEATURESDIR)/raid/data-extra
endif
