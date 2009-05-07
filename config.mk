# System paths
WORKDIR		?= $(CURDIR)/.work
ROOTDIR		?= $(WORKDIR)/root
TOOLSDIR	 = $(CURDIR)/tools
SCRIPTDIR	?= $(CURDIR)/scripts.d
CONFIGDIR	?= $(CURDIR)/config.d
RULESDIR	?= $(CURDIR)/rules.d

# Helpers
GET_VARIABLE		= $(TOOLSDIR)/get-variable
CREATE_INITRD		= $(TOOLSDIR)/create-initrd
PACK_IMAGE		= $(TOOLSDIR)/pack-image
COMPRESS_IMAGE		= $(TOOLSDIR)/compress-image
OPTIONAL_BOOTSPLASH	= $(TOOLSDIR)/optional-bootsplash
ADD_MODULES_ROOTFS	= $(TOOLSDIR)/add-modules-rootfs
ADD_MODULES_IDE		= /bin/true
ADD_MODULES_SCSI	= /bin/true
ADD_MODULES_RAID	= /bin/true

# Global variables
FSTAB			?= /etc/fstab
RAIDTAB			?= /etc/raidtab
FIRMWARE_DIRS		?= /lib/firmware /usr/lib/hotplug/firmware /usr/local/lib/firmware
IGNORE_MODNAMES		?= ppa imm ide_scsi usb_storage

# User defaults
KERNEL		?= $(shell uname -r)
IMAGEFILE	?= /boot/initrd-$(KERNEL).img
COMPRESS	?= gzip

# Load extra configs
-include $(CONFIGDIR)/*.mk
