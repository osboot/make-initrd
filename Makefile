include $(CURDIR)/config.mk

IMAGEFILE	= $(CURDIR)/initrd-$(KERNEL).img
COMPRESS	= gzip
#MODULES_ADD	= ext3 ahci ata_piix ata_generic sd_mod scsi_mod pata_acpi

AUTODETECT	= root net:wlan0

FEATURES = autodetect add-modules compress bootsplash lazy-cleanup

include $(CURDIR)/rules.mk
