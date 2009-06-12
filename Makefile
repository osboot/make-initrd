include $(CURDIR)/config.mk

IMAGEFILE	= $(CURDIR)/initrd-$(KERNEL).img
COMPRESS	= gzip
MODULES_ADD	= ext3 ata_piix sd_mod

FEATURES = add-modules compress optional-bootsplash lazy-cleanup

include $(CURDIR)/rules.mk
