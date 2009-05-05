include $(CURDIR)/config.mk

IMAGE_TYPE	= initramfs
KERNEL		= 2.6.28-lks-wks-alt4
IMAGEFILE	= $(CURDIR)/initrd-$(KERNEL).img
COMPRESS	= bzip2

include $(CURDIR)/rules.mk

initrd: create run-scripts pack compress optional-bootsplash install
