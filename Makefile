include $(CURDIR)/config.mk

KERNEL		= 2.6.28-lks-wks-alt4
IMAGEFILE	= $(CURDIR)/initrd-$(KERNEL).img
COMPRESS	= gzip

include $(CURDIR)/rules.mk

initrd: create add-rootfs-modules run-scripts pack compress optional-bootsplash install
