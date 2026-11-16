# SPDX-License-Identifier: GPL-3.0-or-later
$(call feature-requires,add-modules depmod-image)

KEYBOARD_DEFAULT_MODULES ?= \
	atkbd i8042 usbhid hid-apple hid-sunplus hid-cherry \
	hid-logitech hid-logitech-dj hid-microsoft \
	ehci-hcd ohci-hcd uhci-hcd xhci-hcd hid_generic
