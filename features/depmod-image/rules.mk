depmod-image: create
	@echo "Generating module dependencies in image ..."
	$Q/sbin/depmod -a -F "/boot/System.map-$(KERNEL)" -b $(ROOTDIR) \
		"$(KERNEL)"

pack: depmod-image
