# Guess module: device tree

This feature scans the device tree [1] for compatible nodes and determines which
kernel modules are required for the current hardware. It is intended to be used
in embedded linux environments where device trees are used to describe hardware.

You can disable this feature by adding to the `/etc/initrd.mk`:
```
DISABLE_GUESS += device-tree
```

[1] https://www.devicetree.org/specifications/
[2] https://github.com/devicetree-org/devicetree-specification
