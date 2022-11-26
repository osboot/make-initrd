# Feature: zfs

Feature adds OpenZFS to initramfs.

[OpenZFS](https://github.com/openzfs/zfs) is an advanced file system and volume
manager which was originally developed for Solaris and is now maintained by the
OpenZFS community.

# Parameters

- `root=<pool>/<dataset>` -- (uses this for rpool - first part)
- `root=ZFS=<pool>/<dataset>` -- (uses this for rpool - first part, without 'ZFS=')
- `root=zfs:<pool>/<dataset>` -- (uses this for rpool - first part, without 'zfs:')
- `root=zfs:AUTO` -- (tries to detect both pool and rootfs)
- `zfs_force={1|0}` -- If set to 1, the initramfs will run zpool import -f when
  attempting to import pools if the required pool isn't automatically imported
  by the zfs module. This can save you a trip to a bootcd if hostid has changed,
  but is dangerous and can lead to zpool corruption, particularly in cases where
  storage is on a shared fabric such as iSCSI where multiple hosts can access
  storage devices concurrently. Please understand the implications of
  force-importing a pool before enabling this option!
- `spl_hostid=HEX` -- By default, the hostid used by the SPL module is read from
  `/etc/hostid` inside the initramfs. This file is placed there from the host
  system when the initramfs is built which effectively ties the ramdisk to the
  host which builds it. If a different hostid is desired, one may be set in this
  attribute and will override any file present in the ramdisk. The format should
  be hex exactly as found in the `/etc/hostid` file, IE `spl_hostid=0x00bab10c`.
