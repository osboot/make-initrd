# Feature: rootfs

Feature records basic root filesystem settings for initramfs and generates
`/etc/fstab` for the target root.

During image creation it writes init runtime settings and creates an `fstab`
from the mountpoints known to the build.

## Configuration

- `RUNTIME_QUIET` controls the default init verbosity in initramfs.
- `RUNTIME_DMESG_LOGLEVEL` controls the kernel log level used during early
  boot.

## Guess logic

The feature also adds guessing support for filesystem modules needed by the
detected root filesystem type.
