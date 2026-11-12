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
hardware and mountpoint configuration.

Quite a long time ago, a wonderful sysfs virtual file system appeared in the
linux kernel. It is usually mounted in `/sys`. It is required for many purposes
and is usually always mounted on the system. To guess the hardware
configuration, `make-initrd` parses `/sys` and `/proc` and must have privileges
to read data from there.

Creating an image consists of two parts:

* Creation of internal config based on the user config (`/etc/initrd.mk`).
* Creating an image based on an internal temporary config.

## Detection Architecture

Autodetection is organized directly around feature modules:

* Built-in detection logic is implemented inside feature directories:
  `features/<feature>/guess/`.
* Custom or third-party detection modules can be placed in `guess/<name>/action`
  or `/usr/share/make-initrd/guess/<name>/action`.
* The detection engine (`guess-runner`) runs detection modules in iterative
  batches:
  1. It starts with the initial queue of modules specified by `AUTODETECT`
     (or all available modules if `AUTODETECT = all`).
  2. Each module's `guess/action` script executes and records proposals
     (features, kernel modules, modaliases, configuration variables) into a
     structured provenance log (`guess.tsv`).
  3. When new features are proposed, their declared dependencies
     (`$(call guess-requires, <module>...)` in `config.mk`) dynamically trigger
     further detection modules into the queue.
  4. The loop repeats until all required modules have run and no new dependencies
     are discovered (convergence).

## Step-by-Step Example

To understand how `/sys` is used to find kernel modules, let's take an example and
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

# Autodetection Goals

`make-initrd` provides several CLI commands for inspecting and utilizing autodetection:

## Guess Configuration File (`guess-config`)

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
        gpu-drm network rdshell rootfs system-glibc ucode
```

## Guess Modules (`guess-modules`)

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

## Configuration Provenance & Explainability (`guess-explain`)

To inspect why specific features, kernel drivers, or variables were proposed,
use `guess-explain`:

```bash
$ make-initrd guess-explain
```
```text
Guessed Configuration Provenance:

Features proposed by modules:
  [guess-acpica]
    ├── acpica
  [guess-common]
    ├── add-udev-rules
    ├── cleanup
    ├── compress
  [guess-config]
    ├── add-modules
  [guess-rdshell]
    ├── rdshell
  [guess-rootfs]
    ├── add-udev-rules
    ├── rootfs
  [guess-ucode]
    ├── ucode

Kernel modules / modaliases proposed:
  [depinfo]
    ├── /lib/modules/.../kernel/drivers/hid/hid-generic.ko (group: rescue)
    ├── /lib/modules/.../kernel/drivers/hid/usbhid/usbhid.ko (group: rescue)

Configuration variables set:
  [guess-rootfs]
    ├── MOUNTPOINTS = /
```

# Configuration Parameters

Autodetection behavior can be customized in `/etc/initrd.mk`:

* **`AUTODETECT`**: List of guess modules to run. Default is `all`.
  - Setting `AUTODETECT =` completely disables autodetection (no guess modules
    and no `common` hooks are executed).
  - You can specify exact modules to run, using either canonical feature names
    or legacy short aliases:
    ```make
    AUTODETECT = modules-network rootfs
    # or using aliases:
    AUTODETECT = net root
    ```

* **`DISABLE_GUESS`**: List of guess modules to skip.
  - Supports both canonical names and legacy aliases:
    ```make
    DISABLE_GUESS = modules-network
    # or using alias:
    DISABLE_GUESS = net
    ```
  - Disabling `common` (`DISABLE_GUESS = common`) skips all generic hooks
    provided by `features/*/guess/common`.

* **Feature-specific detection options**:
  - `GUESS_NET_IFACE` (in [modules-network](features/modules-network/README.md)):
    specifies which network interfaces to inspect (`all`, `auto`, or a list of
    interface names).
  - `DEVICES` (in [modules-blockdev](features/modules-blockdev/README.md)):
    specifies explicit block device nodes to detect kernel drivers for.

# Detection Modules and Aliases

The table below lists the built-in detection modules provided by features:

| Canonical Module | Legacy Alias | Description |
|:-----------------|:-------------|:------------|
| [modules-blockdev](features/modules-blockdev/README.md) | `device` | Detects storage drivers for devices listed in `$DEVICES` |
| [modules-device-tree](features/modules-device-tree/README.md) | `device-tree` | Detects hardware drivers from Device Tree (`/sys/firmware/devicetree`) |
| [modules-keyboard](features/modules-keyboard/README.md) | `keyboard` | Detects drivers for input keyboards |
| [modules-network](features/modules-network/README.md) | `net` | Detects kernel drivers for network interfaces (`$GUESS_NET_IFACE`) |
| [modules-touchscreen](features/modules-touchscreen/README.md) | `touchscreen` | Detects touchscreen input drivers |
| [resume](features/resume/README.md) | `resume` | Detects hibernation resume partition or swap device |
| [rootfs](features/rootfs/README.md) | `root` | Detects root filesystem, mountpoints, and underlying storage (LVM, LUKS, RAID, bcache, overlayfs, zfs) |
| [acpica](features/acpica/README.md) | `acpica` | Detects ACPI table support requirements |
| [bootconfig](features/bootconfig/README.md) | `bootconfig` | Detects kernel bootconfig support |
| [lkrg](features/lkrg/README.md) | `lkrg` | Detects Linux Kernel Runtime Guard |
| [qemu](features/qemu/README.md) | `qemu` | Detects execution in QEMU virtual machine environment |
| [rdshell](features/rdshell/README.md) | `rdshell` | Proposes rescue shell support if enabled |
| [smart-card](features/smart-card/README.md) | `smart-card` | Detects smart card reader hardware |
| [ucode](features/ucode/README.md) | `ucode` | Detects CPU microcode requirements (Intel / AMD) |
| `common` | `common` | Virtual module that runs generic detection hooks (`features/*/guess/common`) |

# Feature Detection Hooks

Inside each feature directory (`features/<feature>/guess/`), four types of scripts
can participate in autodetection:

1. **`guess/action`**: The main detection script for the feature, run by `guess-runner`
   when the module is scheduled.
2. **`guess/device`**: Hook script invoked by `rootfs` while traversing parent devices
   in the `/sys` block device hierarchy. Used to detect block layers like LVM,
   LUKS, MDADM, bcache, or devmapper. An optional `guess/device.filter` script can
   define a `device_filter` function to fast-path or skip invoking `guess/device` for
   non-matching sysfs devices without spawning a child process.
3. **`guess/fstype`**: Hook script invoked by `rootfs` to identify filesystem types
   and propose appropriate filesystem drivers (e.g. ext4, btrfs, xfs, zfs, overlayfs).
4. **`guess/common`**: Generic hook script executed during the `common` detection pass
   (e.g., to detect locales, udev rules, or compression preferences).

Detection scripts use shell helper functions provided by `guess-functions`:

* `guess_feature <feature>`: Proposes activating a feature.
* `guess_module [group] <module>`: Proposes adding a kernel module.
* `guess_modalias [group] <modalias>`: Proposes resolving a modalias string.
* `guess_variable <name> <value>`: Proposes setting a configuration variable.
