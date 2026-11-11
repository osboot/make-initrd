# Feature: modules-device-tree

This feature scans the Open Firmware Device Tree [1] for compatible nodes and
determines which kernel modules are required for the current hardware. It is
intended to be used in embedded Linux environments where device trees are used to
describe hardware peripherals.

## Parameters

You can disable autodetection for this feature by adding to `/etc/initrd.mk`:

```make
DISABLE_GUESS += device-tree
```

[1] https://www.devicetree.org/specifications/
