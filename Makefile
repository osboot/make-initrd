include $(CURDIR)/config.mk

IMAGEFILE	= $(CURDIR)/initrd-$(KERNEL).img
COMPRESS	= gzip
#MODULES_ADD	= ext3 ata_piix sd_mod

FEATURES = autodetect-modules compress bootsplash lazy-cleanup

include $(CURDIR)/rules.mk
