# Feature: usb-storage

Feature adds autodetection support for block devices backed by USB storage.

The guess logic checks whether the detected device is driven by `ub` or
`usb-storage` and, if so, requests both kernel modules. When
`CONFIG_USB_LIBUSUAL=y` is present in the kernel config, this helps pull in the
USB mass-storage stack for the root device.
