# Feature: modules-keyboard

This feature detects and includes kernel modules required for keyboard and input
devices (`/sys/class/input`).

## Parameters

You can disable autodetection for this feature by adding to `/etc/initrd.mk`:

```make
DISABLE_GUESS += keyboard
```
