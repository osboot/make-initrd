# System paths
WORKDIR		?= $(CURDIR)/.work
ROOTDIR		?= $(WORKDIR)/root
TOOLSDIR	 = $(CURDIR)/tools
DATADIR		?= $(CURDIR)/data
HELPERSDIR	?= $(TOOLSDIR)
SCRIPTDIR	?= $(CURDIR)/scripts.d
FEATURESDIR	?= $(CURDIR)/features
AUTODETECTDIR	?= $(CURDIR)/autodetect

# Helpers
CREATE_INITRD		= $(TOOLSDIR)/create-initrd
PACK_IMAGE		= $(TOOLSDIR)/pack-image
ADD_MODULE		= $(TOOLSDIR)/add-module
LOAD_MODULE		= $(TOOLSDIR)/load-module
ADD_RESCUE_MODULES	= $(TOOLSDIR)/add-rescue-modules
PUT_FILE		= $(TOOLSDIR)/put-file
PUT_TREE		= $(TOOLSDIR)/put-tree

# Global variables
FIRMWARE_DIRS		?= /lib/firmware /usr/lib/hotplug/firmware /usr/local/lib/firmware
VERBOSE			?=

# User defaults
FEATURES	?=
KERNEL		?= $(shell uname -r)
IMAGEFILE	?= /boot/initrd-$(KERNEL).img
COMPRESS	?= gzip

# Load extra configs
include $(FEATURESDIR)/*/config.mk

ifdef VERBOSE
  verbose=-v
  Q=
else
  Q = @
endif

