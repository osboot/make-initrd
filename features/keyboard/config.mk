KEYBOARD_DEFAULT_MODULES ?= \
	atkbd i8042 usbhid hid-apple hid-sunplus hid-cherry \
	hid-logitech hid-logitech-dj hid-microsoft \
	ehci-hcd ohci-hcd uhci-hcd xhci-hcd hid_generic

GUESSv1_KEYBOARD_SOURCE := $(current-feature-name)
GUESSv1_KEYBOARD_RULE = EXISTS{$(SYSFS_PATH)} ACTION{RUN}=$(FEATURESDIR)/keyboard/bin/guess
