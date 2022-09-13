# Overview

The `make-initrd` tries to guess what the root partition is made of and what
features are used for the loaded system.

Most users don't have a good idea of how the distribution's installer created
the root partition. It is completely unfair to force a user (even an advanced
user) to delve into the nuances of booting the system and linux kernel modules.

The simplest `/etc/initrd.mk` config looks like this:

```make
AUTODETECT = all
```

# How It Works ?

Before actually generating the image, `make-initrd` parses the **current**
configuration.

Quite a long time ago, a wonderful sysfs virtual file system appeared in the
linux kernel. It is usually mounted in `/sys`. It is required for many purposes
and is usually always mounted on the system. To guess the hardware
configuration, `make-initrd` parses `/sys` and must have privileges to read any
data from there.

Creating an image consists of two parts:

* Creation of internal config based on the user config (`/etc/initrd.mk`).
* Creating an image based on an internal temporary config.

After determining the list of devices of interest (paths in `/sys`), the
autodetect process executes guess modules that complete the internal temporary
configuration. These modules are located in the `guess/*` directory. Also,
features can have callbacks to enable certain features.

Some of the guess modules require parameters. These options are also specified
in `/etc/initrd.mk`.

You can read more about individual guess modules [here](GuessConfiguration.md#list-of-modules).

## Step-by-Step Example

To understand how /sys is used to find kernel modules, let's take an example and
replay the steps of guess modules. Let's say, we need to find all kernel modules
that requires a root mountpoint. Again, this is just an illustration of the
sequence of steps.

First of all, we need to determine the device that is mounted as the root
partition:

```bash
$ findmnt -no SOURCE /
/dev/nvme0n1p2
```

Now we need to define the path to this device in `/sys`:

```bash
$ ls -la /sys/class/block/nvme0n1p2
lrwxrwxrwx 1 root root 0 сен 12 11:44 /sys/class/block/nvme0n1p2 -> ../../devices/pci0000:00/0000:00:01.1/0000:01:00.0/nvme/nvme0/nvme0n1/nvme0n1p2
```

Some components can also be symlinks. Therefore, we get an absolute path.

```bash
$ readlink -e /sys/class/block/nvme0n1p2
/sys/devices/pci0000:00/0000:00:01.1/0000:01:00.0/nvme/nvme0/nvme0n1/nvme0n1p2
```

Linux kernel modules are loaded using modalias. Therefore, in the `/sys` tree,
we are primarily interested in the `modalias` file.

```bash
n=/sys/devices/pci0000:00/0000:00:01.1/0000:01:00.0/nvme/nvme0/nvme0n1/nvme0n1p2

while [ "$n" != /sys/devices ]; do
	if [ -e "$n/modalias" ]; then
		cat "$n/modalias"
	fi
	n="${n%/*}"
done
```

For example output is:

```
pci:v000015B7d00005009sv000015B7sd00005009bc01sc08i02
pci:v00001022d00001483sv00001022sd00001234bc06sc04i00
```

Now we have enough information to search for modules:

```bash
$ modprobe -v -D -n pci:v000015B7d00005009sv000015B7sd00005009bc01sc08i02
insmod /lib/modules/5.14.0.162-centos-alt1.el9/kernel/block/t10-pi.ko
insmod /lib/modules/5.14.0.162-centos-alt1.el9/kernel/drivers/nvme/host/nvme-core.ko
insmod /lib/modules/5.14.0.162-centos-alt1.el9/kernel/drivers/nvme/host/nvme.ko

$ modprobe -v -D -n pci:v00001022d00001483sv00001022sd00001234bc06sc04i00
modprobe: FATAL: Module pci:v00001022d00001483sv00001022sd00001234bc06sc04i00 not found in directory /lib/modules/5.14.0
```

So, we need to add modules to the initramfs: `nvme`, `nvme-core` and `t10-pi`.

This is how `make-initrd` looks for kernel modules using `/sys`.

## Guess Configuration File

Using guess modules, you can generate a minimal config for your system:

```bash
$ make-initrd -D guess-config
```
```make
MOUNTPOINTS = \
         /

MODULES_ADD += \
        nvme ext4

FEATURES += \
        add-modules add-udev-rules buildinfo cleanup compress \
        gpu-drm kbd network rdshell rootfs system-glibc ucode
```

## Guess Modules

You can also see what modules are required for any mountpoint or block device.

```bash
$ make-initrd -D guess-modules /
/lib/modules/5.14.0/kernel/arch/x86/crypto/crc32c-intel.ko
/lib/modules/5.14.0/kernel/block/t10-pi.ko
/lib/modules/5.14.0/kernel/drivers/nvme/host/nvme-core.ko
/lib/modules/5.14.0/kernel/drivers/nvme/host/nvme.ko
/lib/modules/5.14.0/kernel/fs/ext4/ext4.ko
/lib/modules/5.14.0/kernel/fs/jbd2/jbd2.ko
/lib/modules/5.14.0/kernel/fs/mbcache.ko
```

# List of Modules

Below is a list of guess modules.
