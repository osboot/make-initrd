# System paths
WORKDIR		?= $(CURDIR)/.work
ROOTDIR		?= $(WORKDIR)/root
TOOLSDIR	 = $(CURDIR)/tools
DATADIR		?= /usr/share/initramfs-data/data
HELPERSDIR	?= /usr/share/initramfs-data/tools
SCRIPTDIR	?= $(CURDIR)/scripts.d
CONFIGDIR	?= $(CURDIR)/config.d
RULESDIR	?= $(CURDIR)/rules.d

# Helpers
GET_VARIABLE		= $(TOOLSDIR)/tool-runner get-variable
CREATE_INITRD		= $(TOOLSDIR)/tool-runner create-initrd
PACK_IMAGE		= $(TOOLSDIR)/tool-runner pack-image
COMPRESS_IMAGE		= $(TOOLSDIR)/tool-runner compress-image
OPTIONAL_BOOTSPLASH	= $(TOOLSDIR)/tool-runner optional-bootsplash
ADD_MODULE		= $(TOOLSDIR)/tool-runner add-module
LOAD_MODULE		= $(TOOLSDIR)/tool-runner load-module
PUT_FILE		= $(TOOLSDIR)/tool-runner put-file


# Global variables
FSTAB			?= /etc/fstab
RAIDTAB			?= /etc/raidtab
FIRMWARE_DIRS		?= /lib/firmware /usr/lib/hotplug/firmware /usr/local/lib/firmware
MODULES_ADD		?=
MODULES_LOAD		?=
VERBOSE			?=

# User defaults
FEATURES	?=
KERNEL		?= $(shell uname -r)
IMAGEFILE	?= /boot/initrd-$(KERNEL).img
COMPRESS	?= gzip

# Load extra configs
-include $(CONFIGDIR)/*.mk

ifdef VERBOSE
  verbose=-v
  Q=
else
  Q = @
endif

