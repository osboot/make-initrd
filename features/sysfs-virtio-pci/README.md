# Feature: sysfs-virtio-pci

Feature adds autodetection support for devices backed by the `virtio-pci` bus.

When the guess path reaches `/devices/virtio-pci`, the feature requests the
`virtio_pci` kernel module.
