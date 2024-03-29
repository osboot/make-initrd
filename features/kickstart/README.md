# Feature: kickstart

Feature adds the ability to run kickstart-script from initramfs. The kickstart
looks similar to the kickstart implementation in RedHat's anaconda.

## Boot parameters

- `ksfile=<FILE>` - Specifies the full path to the script.

## Kickstart

The kickstart is designed to automate some operations before the system boots.

### Kickstart file

The kickstart file is a simple text file, containing a list of items, each
identified by a keyword.

First, be aware of the following issues when you are creating your kickstart
file:

* There is a natural order for commands should be followed.
* The `%pre`, `%pre-install`, `%post` and `%onerror` sections – These sections
  can be in any order.
* Lines starting with a pound sign (`#`) are treated as comments and are ignored.

### Recommended System Swap Space

The table below provides the recommended size of a swap partition depending on
the amount of RAM in your system.

|Amount of RAM   | Recommended swap space      | Recommended swap space       |
|in the system   |                             | if allowing for hibernation  |
|----------------|-----------------------------|------------------------------|
|less than 2 GB  | 2 times the amount of RAM   | 3 times the amount of RAM    |
|2 GB - 8 GB     | Equal to the amount of RAM  | 2 times the amount of RAM    |
|8 GB - 64 GB    | 0.5 times the amount of RAM | 1.5 times the amount of RAM  |
|more than 64 GB | workload dependent          | 1.5 times the amount of RAM  |

### Kickstart commands

The following commands can be placed in a kickstart file.

* Storage and Partitioning
  - [ignoredisk](#ignoredisk)
  - [clearpart](#clearpart)
  - [reqpart](#reqpart)
  - [makefs](#makefs)
  - [crypto](#crypto)
  - [part or partition](#part-or-partition)
  - [volgroup](#volgroup)
  - [logvol](#logvol)
  - [raid](#raid)
  - [btrfs](#btrfs)
* Installation Methods and Sources
  - [liveimg](#liveimg)
* After the Installation
  - [reboot](#reboot)
  - [halt](#halt)
  - [poweroff](#poweroff)
  - [shutdown](#shutdown)

#### ignoredisk
```
ignoredisk [--drives=IGNOREDISK] [--only-use=ONLYUSE]
```
Controls kickstart's access to disks attached to the system. By default, all
disks will be available for partitioning. Only one of the following three
options may be used.

Options:

* `--drives=IGNOREDISK` - Specifies those disks that anaconda should not touch
  when partitioning, formatting, and clearing.

* `--only-use=ONLYUSE` - Specifies the opposite - only disks listed here will be
  used during installation.


#### clearpart
```
clearpart [--all] [--drives=DRIVES] [--initlabel] [--none] [--list=DEVICES] [--disklabel=DISKLABEL]
```
Removes partitions from the system, prior to creation of new partitions. By
default, no partitions are removed.

Options:

* `--all` - Erases all partitions from the system.

* `--drives=DRIVES` - Specifies which drives to clear partitions from.

* `--initlabel` - Initializes the disk label to the default for your architecture.

* `--none` - Do not remove any partitions. This is the default.

* `--list=DEVICES` - Specifies which partitions to clear.

* `--disklabel=DISKLABEL` - Set the default disklabel to use. Only disklabels
  supported for the platform will be accepted. eg. dos and gpt.


#### reqpart
```
reqpart [--add-boot]
```
Automatically create partitions required by your hardware platform. These
include a `/boot/efi` for x86_64 and Aarch64 systems with UEFI firmware,
`biosboot` for x86_64 systems with BIOS firmware and GPT.

Options:

* `--add-boot` - Create a separate /boot partition in addition to the
  platform-specific partition created by the base command.


#### makefs
```
makefs <device> [--fstype=FSTYPE] [--label=LABEL]
```
Creates or replaces the filesystem on the device.

Options:

* `--fstype=FSTYPE` - Sets the filesystem type for the device (default: ext4).

* `--label=LABEL` - Specifies the label to give to the filesystem to be made.


#### crypto
```
crypto [--name=NAME] [--passphrase=PASSPHRASE] [--cipher=CIPHER] [--pbkdf=PBKDF]
   [--pbkdf-memory=MEMORY] [--pbkdf-time=TIME] [--pbkdf-iterations=ITERATIONS]
   [--passfile=FILE]
   <device>
```
Creates LUKS encrypted volume.

Options:

* `--passphrase=PASSPHRASE` - Specifies the passphrase to use when encrypting
  this device.

* `--passfile=FILE` - Specifies the file that contains the password.

* `--cipher=CIPHER` - Specifies which encryption algorithm should be used to
  encrypt the filesystem.

* `--pbkdf=PBKDF` - Sets Password-Based Key Derivation Function (PBKDF)
  algorithm for LUKS keyslot.

* `--pbkdf-memory=MEMORY` - Sets the memory cost for PBKDF.

* `--pbkdf-time=TIME` - Sets the number of milliseconds to spend with PBKDF
  passphrase processing.

* `--pbkdf-iterations=ITERATIONS` - Sets the number of iterations directly and
  avoids PBKDF benchmark.


#### part or partition
```
part [--ondisk=DISK] [--onpart=ONPART] [--asprimary]
   [--fstype=FSTYPE] [--label=LABEL]
   [--useexisting] [--noformat] [--hibernation]
   [--encrypted] [--passphrase=PASSPHRASE] [--passfile=FILE] [--cipher=CIPHER]
   [--pbkdf=PBKDF] [--pbkdf-memory=MEMORY] [--pbkdf-time=TIME] [--pbkdf-iterations=ITERATIONS]
   <mntpoint>
```
Creates a partition on the system.

The `<mntpoint>` is where the partition will be mounted and must be of one of
the following forms:

* `/<path>` For example, /, /usr, /home

* `swap` The partition will be used as swap space.

* `raid.<id>` The partition will be used for software RAID. Refer to the raid command.

* `pv.<id>` The partition will be used for LVM. Refer to the logvol command.
* `zfs.<id>`, `zfs.boot.<id>`, `zfs.mirror.<id>`, `zfs.raidz.<id>` The partition
  will be used for ZFS filesystem. `boot`, `mirror`, `raidz` reflect the type of
  partition being created.

Options:

* `--ondisk=DISK`, `--ondrive=DISK` - Forces the partition to be created on
  a particular disk.

* `--onpart=ONPART`, `--usepart=ONPART` - Puts the partition on an already
  existing device.

* `--asprimary` Forces automatic allocation of the partition as a primary
  partition or the partitioning will fail. The `--asprimary` option only makes
  sense with the MBR partitioning scheme and is ignored when the GPT
  partitioning scheme is used.

* `--size=SIZE` - Size of this partition.

* `--grow` - Tells the partition to grow to fill available space (if any).

* `--resize` - Attempt to resize this partition to the size given by `--size=`.

* `--fsoptions=FSOPTS` - Specifies a free form string of options to be used when
  mounting the filesystem. This string will be copied into the fstab file.

* `--useexisting`, `--noformat` - Use an existing logical volume and do not
  format it.

* `--encrypted` - Specifies that this logical volume should be encrypted.

* `--passphrase=PASSPHRASE`, `--passfile=FILE`, `--cipher=CIPHER`, `--pbkdf=PBKDF`,
  `--pbkdf-memory=MEMORY`, `--pbkdf-time=TIME`, `--pbkdf-iterations=ITERATIONS` -
  See options for the [crypto](#crypto) command.

* `--fstype=FSTYPE`, `--label=LABEL` - See options for the [makefs](#makefs) command.

* `--hibernation` - This option can be used to automatically determine the size
  of the swap partition big enough for hibernation if no size is specified with
  the `--size=` option. If this option is specified and `--size` is not defined,
  then the size will be determined according to the recommendations.
  See [Recommended System Swap Space](#recommended-system-swap-space).


#### volgroup
```
volgroup [--pesize=PESIZE] <name> <partitions...>
```
Creates a Logical Volume Management (LVM) group.

`<name>` Name given to the volume group.

`<partitions>` Physical Volume partitions to be included in this Volume Group.

Options:

* `--pesize=PESIZE` - Sets the size of the physical extents in KiB.


#### logvol
```
logvol --vgname=VGNAME --name=NAME [--chunksize=SIZE] [--percent=PERCENT]
   [--size=SIZE] [--grow] [--resize] [--fsoptions=FSOPTS]
   [--fstype=FSTYPE] [--label=LABEL]
   [--useexisting] [--noformat] [--hibernation]
   [--encrypted] [--passphrase=PASSPHRASE] [--passfile=FILE] [--cipher=CIPHER]
   [--pbkdf=PBKDF] [--pbkdf-memory=MEMORY] [--pbkdf-time=TIME] [--pbkdf-iterations=ITERATIONS]
   <mntpoint>
```
Create a logical volume for Logical Volume Management (LVM).

`<mntpoint>` Mountpoint for this logical volume or ‘none’.

Options:

* `--vgname=VGNAME` - Name of the Volume Group this logical volume belongs to.

* `--name=NAME` - The name of this logical volume.

* `--chunksize=SIZE` - Specifies the chunk size (in KiB) for a new thin pool device.

* `--size=SIZE` - Size of this logical volume.

* `--grow` - Tells the logical volume to grow to fill available space (if any).

* `--resize` - Attempt to resize this logical volume to the size given by `--size=`.

* `--fsoptions=FSOPTS` - Specifies a free form string of options to be used when
  mounting the filesystem. This string will be copied into the fstab file.

* `--useexisting`, `--noformat` - Uses an existing logical volume and do not
  format it.

* `--encrypted` - Specifies that this logical volume should be encrypted.

* `--passphrase=PASSPHRASE`, `--passfile=FILE`, `--cipher=CIPHER`, `--pbkdf=PBKDF`,
  `--pbkdf-memory=MEMORY`, `--pbkdf-time=TIME`, `--pbkdf-iterations=ITERATIONS` -
  See options for the [crypto](#crypto) command.

* `--fstype=FSTYPE`, `--label=LABEL` - See options for the [makefs](#makefs) command.

* `--hibernation` - This option can be used to automatically determine the size
  of the swap partition big enough for hibernation if no size is specified with
  the `--size=` option. If this option is specified and `--size` is not defined,
  then the size will be determined according to the recommendations.
  See [Recommended System Swap Space](#recommended-system-swap-space).


#### raid
```
raid --device=DEVICE [--level=LEVEL] [--chunksize=SIZE] [--fsoptions=FSOPTS]
   [--fstype=FSTYPE] [--label=LABEL]
   [--useexisting] [--noformat] [--hibernation]
   [--encrypted] [--passphrase=PASSPHRASE] [--passfile=FILE] [--cipher=CIPHER]
   [--pbkdf=PBKDF] [--pbkdf-memory=MEMORY] [--pbkdf-time=TIME] [--pbkdf-iterations=ITERATIONS]
   <mntpoint> <partitions...>
```
Assembles a software RAID device.

`<mntpoint>` Location where the RAID file system is mounted.

`<partitions>` The software raid partitions lists the RAID identifiers to add
to the RAID array.

Options:

* `--device=DEVICE` - Name of the RAID device to use (such as ‘md0’ or ‘home’).

* `--level=LEVEL` - RAID level to use eg. 0, 1, 2, 3, 4, 5.

* `--chunksize=SIZE` - Specifies chunk size of kilobytes. The default when
  creating an array is 512KB.

* `--fsoptions=FSOPTS` - Specifies a free form string of options to be used when
  mounting the filesystem. This string will be copied into the fstab file.

* `--useexisting`, `--noformat` - Uses an existing logical volume and do not
  format it.

* `--encrypted` - Specifies that this logical volume should be encrypted.

* `--passphrase=PASSPHRASE`, `--passfile=FILE`, `--cipher=CIPHER`, `--pbkdf=PBKDF`,
  `--pbkdf-memory=MEMORY`, `--pbkdf-time=TIME`, `--pbkdf-iterations=ITERATIONS` -
  See options for the [crypto](#crypto) command.

* `--fstype=FSTYPE`, `--label=LABEL` - See options for the [makefs](#makefs) command.

* `--hibernation` - This option can be used to automatically determine the size
  of the swap partition big enough for hibernation if no size is specified with
  the `--size=` option. If this option is specified and `--size` is not defined,
  then the size will be determined according to the recommendations.
  See [Recommended System Swap Space](#recommended-system-swap-space).


#### btrfs
```
btrfs [--subvol] [--data=LEVEL] [--metadata=LEVEL] [--label=LABEL]
   [--fsoptions=FSOPTS] [--name=NAME]
   [--useexisting] [--noformat]
   <mntpoint> <partitions...>
```
Create a Btrfs volume or subvolume.

`<mntpoint>` Location where the file system is mounted.
`<partitions...>` lists the BTRFS identifiers to add to the BTRFS volume.

Options:

* `--subvol` Create BTRFS subvolume.

* `--name=NAME` Subvolume name.

* `--data=LEVEL` RAID level to use (0, 1, 10) for filesystem data.

* `--metadata=LEVEL` RAID level to use (0, 1, 10) for filesystem/volume
  metadata.

* `--label=LABEL` Specify the label to give to the filesystem to be made.

* `--fsoptions=FSOPTS` - Specifies a free form string of options to be used when
  mounting the filesystem. This string will be copied into the fstab file.

* `--useexisting`, `--noformat` - Uses an existing logical volume and do not
  format it.

Example:
```bash
part btrfs.01 --ondisk=sda --size=100%
part btrfs.02 --ondisk=sdb --size=100%
part btrfs.03 --ondisk=sdc --size=100%
part btrfs.04 --ondisk=sdd --size=100%

btrfs none --data=1 --metadata=1 --label=fs btrfs.0*

btrfs /     --subvol --name=root LABEL=fs
btrfs /home --subvol --name=home LABEL=fs
```

#### zpool
```
zpool [--feature=VALUE ...] [--property=VALUE ...] [--fsoption=FSOPT ...] <pool>
    (<partitions...>|<vdev...>)
```

Creates a new storage pool containing the virtual devices specified on the
command line. The pool names `mirror`, `raidz`, `draid`, `spare` and `log` are
reserved. Using the same device in two pools will result in pool corruption.

`<pool>` the name of zfs pool.
`<partitions...>` lists the `zfs.<id>` identifiers to add to the ZFS pool or
real partitons.

Example:
```bash
# Creating a RAID-Z Storage Pool
zpool tank1 raidz sda sdb sdc sdd sde sdf

# Creating a Mirrored Storage Pool
zpool tank2 mirror sda sdb mirror sdc sdd
```

Options:

* `--feature=VALUE` Sets the given pool feature. See the `zpool-features(7)`
  section for a list of valid features that can be set. Value can be either
  disabled or enabled.
* `--property=VALUE` Sets the given pool properties. See `zpoolprops(7)` for a
  list of valid properties that can be set.
* `--fsoption=FSOPT` Sets the given file system properties in the root file
  system of the pool. See zfsprops(7) for a list of valid properties that can
  be set.

Example:
```bash
part zfs.11 --size=30%
part zfs.21 --size=70%

part zfs.12 --size=30%
part zfs.22 --size=70%

zpool rpool mirror zfs.1*
zpool dpool mirror zfs.2*
```

#### zfs
```
zfs [--property=VALUE ...] [--size=VALUE] [--sparse] (<mntpoint>|none) <pool>/<volume>
```

Creates a new ZFS file system.

`<mntpoint>` Location where the file system is mounted.

Options:

* `--property=VALUE` Sets the specified property.
* `--size=VALUE` Creates a volume of the given size.
* `--sparse` Creates a sparse volume with no reservation.

Example:
```bash
part zfs.11 --size=30%
part zfs.21 --size=70%

part zfs.12 --size=30%
part zfs.22 --size=70%

zpool rpool mirror zfs.1*
zpool dpool mirror zfs.2*

zfs /    rpool/ROOT
zfs /usr dpool/DATA
```

#### liveimg
```
liveimg --url=<url> [--proxy=<proxyurl>] [--noverifyssl] [--checksum=<sha256>]
```
Install a disk image instead of packages. The URL may also point to a tarfile of
the root filesystem. The file must end in .tar, .tbz, .tgz, .txz, .tar.bz2,
.tar.gz, .tar.xz, .tar.zst, .tar.lz4, .zip, .cpio.

Option:

* `--url=<url>` - The URL to install from. http, https and file are supported.
* `--proxy=<proxyurl>` - Specifies an HTTP/HTTPS proxy to use while performing
  the install. The various parts of the argument act like you would expect.
* `--noverifyssl` - Don't check the server certificate against the available
  certificate authorities.
* `--checksum=<sha256>` - Check sha256 checksum of the image file.

Example:
```bash
liveimg --url=file:///path/to/hasher.tar --checksum=01ba4719c80b6fe911b091a7c05124b64eeece964e09c058ef8f9805daca546b
```

#### reboot
```
reboot [--eject] [--kexec]
```
Reboot after the installation is complete. Normally, kickstart displays a
message and waits for the user to press a key before rebooting.

Options:

* `--eject` Attempt to eject CD or DVD media before rebooting.

* `--kexec` Use kexec to reboot into the new system, bypassing BIOS/Firmware and
  bootloader.


#### poweroff
```
poweroff [--eject] [--kexec]
```

Turn off the machine after the installation is complete. Normally, kickstart
displays a message and waits for the user to press a key before rebooting.

See options for the [reboot](#reboot) command.


#### shutdown
```
shutdown [--eject] [--kexec]
```

At the end of installation, shut down the machine. This is the same as the
poweroff command. Normally, kickstart displays a message and waits for the user
to press a key before rebooting.

See options for the [reboot](#reboot) command.


#### halt
```
halt [--eject] [--kexec]
```

At the end of installation, display a message and wait for the user to press a
key before rebooting. This is the default action.

See options for the [reboot](#reboot) command.

