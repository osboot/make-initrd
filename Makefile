include $(CURDIR)/config.mk

IMAGEFILE	= $(CURDIR)/initrd-$(KERNEL).img
COMPRESS	= gzip
MODULES_ADD	= ext3 ata_piix sd_mod

include $(CURDIR)/rules.mk

pack: add-ls

add-ls:
	$(PUT_PROGRAM) /bin/ls

initrd: create run-scripts pack compress optional-bootsplash install
