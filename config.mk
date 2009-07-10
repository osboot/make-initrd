# System paths
WORKDIR		?= $(CURDIR)/.work
ROOTDIR		?= $(WORKDIR)/root
TOOLSDIR	 = $(CURDIR)/tools
DATADIR		?= $(CURDIR)/data
HELPERSDIR	?= $(TOOLSDIR)
SCRIPTDIR	?= $(CURDIR)/scripts.d
FEATURESDIR	?= $(CURDIR)/features

# Helpers
GET_VARIABLE		= $(TOOLSDIR)/tool-runner get-variable
CREATE_INITRD		= $(TOOLSDIR)/tool-runner create-initrd
PACK_IMAGE		= $(TOOLSDIR)/tool-runner pack-image
ADD_MODULE		= $(TOOLSDIR)/tool-runner add-module
LOAD_MODULE		= $(TOOLSDIR)/tool-runner load-module
ADD_RESCUE_MODULES	= $(TOOLSDIR)/tool-runner add-rescue-modules
PUT_FILE		= $(TOOLSDIR)/tool-runner put-file
PUT_TREE		= $(TOOLSDIR)/tool-runner put-tree

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

