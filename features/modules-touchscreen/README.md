# Feature: modules-touchscreen

This feature detects and includes kernel modules required for touchscreen
devices.

Touchscreen modules may be necessary, for example, if you need to enter a
password to unlock an encrypted mobile device.

## Parameters

You can disable autodetection for this feature by adding to `/etc/initrd.mk`:

```make
DISABLE_GUESS += touchscreen
```
