# Features

A feature is a piece of functionality that can optionally be added to the image.
Some features have auto-detection and are added automatically.

- [add-modules](../features/add-modules/README.md) adds modules to initramfs.
- [bootsplash](../features/bootsplash/README.md) adds bootsplash to initramfs.
- [btrfs](../features/btrfs/README.md) adds btrfs to initramfs.
- [buildinfo](../features/buildinfo/README.md) saves information about the
  created initramfs image.
- [cleanup](../features/cleanup/README.md) clean the working directory and
  the temporary files after the image is created.
- [compress](../features/compress/README.md) compresses initramfs image.
- [depmod-image](../features/depmod-image/README.md) makes depmod inside
  the image.
- [devmapper](../features/devmapper/README.md) adds modules and utilities to
  setup device-mapper in initramfs.
- [kbd](../features/kbd/README.md) adds keymap and console fonts to initramfs.
- [luks](../features/luks/README.md) adds the ability to boot from LUKS
  partition.
- [lvm](../features/lvm/README.md) adds the ability to boot from LVM volume.
- [mdadm](../features/mdadm/README.md) adds the ability to boot from RAID using
  mdadm.
- [modules-crypto-user-api](../features/modules-crypto-user-api/README.md) adds
  a collection of Crypto API modules available to the kernel.
- [modules-filesystem](../features/modules-filesystem/README.md) adds
  a collection of filesystem modules available to the kernel.
- [modules-network](../features/modules-network/README.md) adds a collection
  of network modules available to the kernel.
- [modules-nfs](../features/modules-nfs/README.md) adds a collection of NFS
  modules available to the kernel.
- [modules-virtio](../features/modules-virtio/README.md) adds a collection
  of virtio modules available to the kernel.
- [multipath](../features/multipath/README.md) adds the ability to boot using
  device mapper multipath.
- [network](../features/network/README.md) gives the ability to configure
  network interfaces in initramfs.
- [nfsroot](../features/nfsroot/README.md) adds the ability to mount the root
  via NFS.
- [plymouth](../features/plymouth/README.md) adds plymouth to initramfs.
- [qemu](../features/qemu/README.md) adds modules required to boot inside qemu.
- [raid](../features/raid/README.md) adds the ability to boot from RAID using
  raid autodetect.
- [rdshell](../features/rdshell/README.md) gives the ability to get shell in
  initrd.
- [syslog](../features/syslog/README.md) adds syslog daemon to initramfs.
- [system-glibc](../features/system-glibc/README.md) add additional glibc
  components to the initramfs.
- [ucode](../features/ucode/README.md) adds CPU microcode loading at early
  stage before initramfs.
- [usb](../features/usb/README.md) adds modules required to initialize the USB
  to initramfs.
