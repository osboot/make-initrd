include $(CURDIR)/config.mk

KERNEL		= 2.6.28-lks-wks-alt4
IMAGEFILE	= $(CURDIR)/initrd-$(KERNEL).img
COMPRESS	= gzip
MODULES_ADD	= ext3 ata_piix sd_mod

include $(CURDIR)/rules.mk

initrd: create run-scripts pack compress optional-bootsplash install
