# Feature: usb-storage

Feature adds autodetection support for block devices backed by USB storage.

The guess logic checks whether the detected device is driven by `usb-storage`
or by `uas` (USB Attached SCSI) and, if so, requests that kernel module for the
image.
