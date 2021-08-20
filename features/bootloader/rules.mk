bootloader: create
	@echo "bootloader" > "$(ROOTDIR)/etc/initrd/method"

pack: bootloader

PUT_FEATURE_DIRS  += $(BOOTLOADER_DATADIR)
PUT_FEATURE_PROGS += $(BOOTLOADER_PROGS)
