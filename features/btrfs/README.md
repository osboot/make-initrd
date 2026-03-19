# Feature: btrfs

Feature adds btrfs to initramfs.

## Multi-device support

For multi-device btrfs filesystems (RAID), the feature waits for all component
devices to appear before allowing mount. If some devices are missing after the
timeout, the `degraded` mount option is added automatically so the filesystem
can be mounted with available devices.

## Boot parameters

- `btrfs-member-delay=<SECS>` - determines the number of seconds to wait after
  the last btrfs member device appears before mounting in degraded mode
  (default: 10 seconds). If this value is zero, it means disabling the degraded
  mount support.
