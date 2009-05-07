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

# Global variables
MODPROBE_TYPE		= $(shell $(GET_VARIABLE) modprobe-type)
FSTAB			?= /etc/fstab
RAIDTAB			?= /etc/raidtab

# User defaults
KERNEL		?= $(shell uname -r)
IMAGEFILE	?= /boot/initrd-$(KERNEL).img
COMPRESS	?= gzip

# Load extra configs
-include $(CONFIGDIR)/*.mk
