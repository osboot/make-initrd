# Feature: resume

This feature detects and handles resuming the system from suspend-to-disk
(hibernation). It finds modules needed to access the hibernation image device,
which is useful when the resume device is encrypted or in an unexpected location.

## Parameters

You can disable autodetection for this feature by adding to `/etc/initrd.mk`:

```make
DISABLE_GUESS += resume
```
