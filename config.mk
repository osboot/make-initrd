# System paths
WORKDIR		?= $(CURDIR)/.work
ROOTDIR		?= $(WORKDIR)/root
DATADIR		?= $(CURDIR)/data
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
ADD_MODULE		= $(TOOLSDIR)/add-module
LOAD_MODULE		= $(TOOLSDIR)/load-module
PUT_PROGRAM		= $(TOOLSDIR)/put-program


# Global variables
FSTAB			?= /etc/fstab
RAIDTAB			?= /etc/raidtab
FIRMWARE_DIRS		?= /lib/firmware /usr/lib/hotplug/firmware /usr/local/lib/firmware
IGNORE_MODNAMES		?= ppa imm ide_scsi usb_storage
MODULES_ADD		?=
MODULES_LOAD		?=

# User defaults
KERNEL		?= $(shell uname -r)
IMAGEFILE	?= /boot/initrd-$(KERNEL).img
COMPRESS	?= gzip

# Load extra configs
-include $(CONFIGDIR)/*.mk
