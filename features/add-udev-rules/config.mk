PUT_UDEV_RULES ?=

SYSTEM_UDEV_RULES := \
	$(wildcard /etc/udev/initramfs-rules.d/*.rules) \
	$(wildcard /lib/udev/initramfs-rules.d/*.rules) \
	$(wildcard $(FEATURESDIR)/add-udev-rules/rules.d/*.rules)
