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

### Kickstart commands

The following commands can be placed in a kickstart file.

* Storage and Partitioning
  - [ignoredisk](#ignoredisk)
  - [clearpart](#clearpart)
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
   <device>
```
Creates LUKS encrypted volume.

Options:

* `--passphrase=PASSPHRASE` - Specifies the passphrase to use when encrypting
  this device.

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
   [--useexisting] [--noformat]
   [--encrypted] [--passphrase=PASSPHRASE] [--cipher=CIPHER] [--pbkdf=PBKDF]
   [--pbkdf-memory=MEMORY] [--pbkdf-time=TIME] [--pbkdf-iterations=ITERATIONS]
   <mntpoint>
```
Creates a partition on the system.

The `<mntpoint>` is where the partition will be mounted and must be of one of
the following forms:

* `/<path>` For example, /, /usr, /home

* `swap` The partition will be used as swap space.

* `raid.<id>` The partition will be used for software RAID. Refer to the raid command.

* `pv.<id>` The partition will be used for LVM. Refer to the logvol command.

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

* `--passphrase=PASSPHRASE`, `--cipher=CIPHER`, `--pbkdf=PBKDF`,
  `--pbkdf-memory=MEMORY`, `--pbkdf-time=TIME`, `--pbkdf-iterations=ITERATIONS` -
  See options for the [crypto](#crypto) command.

* `--fstype=FSTYPE`, `--label=LABEL` - See options for the [makefs](#makefs) command.


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
   [--useexisting] [--noformat]
   [--encrypted] [--passphrase=PASSPHRASE] [--cipher=CIPHER] [--pbkdf=PBKDF]
   [--pbkdf-memory=MEMORY] [--pbkdf-time=TIME] [--pbkdf-iterations=ITERATIONS]
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

* `--passphrase=PASSPHRASE`, `--cipher=CIPHER`, `--pbkdf=PBKDF`,
  `--pbkdf-memory=MEMORY`, `--pbkdf-time=TIME`, `--pbkdf-iterations=ITERATIONS` -
  See options for the [crypto](#crypto) command.

* `--fstype=FSTYPE`, `--label=LABEL` - See options for the [makefs](#makefs) command.


#### raid
```
raid --device=DEVICE [--level=LEVEL] [--chunksize=SIZE] [--fsoptions=FSOPTS]
   [--fstype=FSTYPE] [--label=LABEL]
   [--useexisting] [--noformat]
   [--encrypted] [--passphrase=PASSPHRASE] [--cipher=CIPHER] [--pbkdf=PBKDF]
   [--pbkdf-memory=MEMORY] [--pbkdf-time=TIME] [--pbkdf-iterations=ITERATIONS]
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

* `--passphrase=PASSPHRASE`, `--cipher=CIPHER`, `--pbkdf=PBKDF`,
  `--pbkdf-memory=MEMORY`, `--pbkdf-time=TIME`, `--pbkdf-iterations=ITERATIONS` -
  See options for the [crypto](#crypto) command.

* `--fstype=FSTYPE`, `--label=LABEL` - See options for the [makefs](#makefs) command.


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


#### liveimg
```
liveimg --url=<url> [--proxy=<proxyurl>] [--noverifyssl] [--checksum=<sha256>]
```
Install a disk image instead of packages. The URL may also point to a tarfile of
the root filesystem. The file must end in .tar, .tbz, .tgz, .txz, .tar.bz2,
.tar.gz, .tar.xz, .zip, .cpio.

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

