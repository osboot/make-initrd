# Feature: bootconfig

Feature adds Extra Boot Config (XBC) support.

The boot configuration expands the current kernel command line to support
additional key-value data when booting the kernel in an efficient way. This
allows administrators to pass a structured-Key config file.

See https://www.kernel.org/doc/Documentation/admin-guide/bootconfig.rst for further infomation how it works.

## Parameters

- **BOOT_CONFIG** -- Defines the path to the config that needs to be added to the initramfs.
- **BOOT_CONFIG_PROG** -- Specifies the custom location of `bootconfig` utility. By
  default, the utility will be searched in `PATH`.
