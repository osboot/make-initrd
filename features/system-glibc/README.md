# Feature: system-glibc

This is a system feature. Its purpose is to add additional glibc components to the initramfs image for which there are no direct dependencies such as `libgcc_s.so` or `libnss_files.so`.

## Parameters

- **INITRD_NO_LIBNSS** -- Do not pack `libnss_*` libraries;
- **INITRD_NO_LIBGCC_S** -- Do not pack `libgcc_s` library;
- **INITRD_NO_PWGR** -- Do not generate `/etc/passwd` and `/etc/group` based on system files;
- **INITRD_NO_NETWORK** -- Do not pack `/etc/resolv.conf` and `/etc/host.conf`.
