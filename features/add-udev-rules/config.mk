PUT_UDEV_RULES ?= \
	$(wildcard $(FEATURESDIR)/add-udev-rules/rules.d/*.rules) \
	$(wildcard /etc/udev/initramfs-rules.d/*.rules) \
	$(wildcard /lib/udev/initramfs-rules.d/*.rules)
