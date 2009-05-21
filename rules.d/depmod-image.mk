ifndef _DEPMOD_IMAGE_MK
_DEPMOD_IMAGE_MK = 1

depmod-image: create
	@echo "Generating module dependencies in image ..."
	$Q/sbin/depmod -a -F "/boot/System.map-$(KERNEL)" -b $(ROOTDIR) \
		"$(KERNEL)"

pack: depmod-image

endif
