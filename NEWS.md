Version 2.22.0

- Runtime:
  + ueventd: Process events that were already in the queue before the daemon
    startup. This is a fix initramfs boot if ueventd is started after udevd due
    to dependencies.
- Feature rootfs:
  + Add fstype kernel module only if module exists. Some filesystems cannot be
    built as modules. For example tmpfs is neither a builtin nor a loadable module.

Version 2.21.0

- Runtime:
  + Ignore errors if we were unable to send udevd a signal to exit.
  + Mount /proc without writing in /etc/mtab.
- Feature compress:
  + Pass --best instead of -9 to compressors.
  + Force xz to use crc32 polynomial for integrity check.
  + Add zstd and lz4 support.
- Feature gpu-drm:
  + Filter enabled and/or connected drm devices.
- Utilities:
  + initrd-put: Check shebang only in an executable file.
  + initrd-put: Fix false warnings from libelf.

Version 2.20.1

- Feature gpu-drm:
  + Drop prefix from output which resulted in an image build error.
- Feature smart-card:
  + Fixes for smart-card feature: remove odd quotes around opensc-pkcs11.so string.
- Guess subsystem:
  + Rename guess/sshfsroot to guess/smart-card.

Version 2.20.0

- Runtime:
  + Always create /root and /home/root as a directories.
  + udev: Remove start dependency on uevent service.
- New feature:
  + Add smart-card feature. Feature adds smart card daemon and smart card
    utilities.
- Feature smart-card:
  + Add ability to put pkcs#11 modules into initrd.
- Feature sshfsroot:
  + Add plymouth support.
  + Enable asking passphrase.
  + Detect smart-card feature dependency according ssh config file.
- Feature kbd:
  + Require gpu-drm if framebuffer is used on vtconsole.
- Feature kickstart:
  + Unpause ueventd queues after KSFILE processing.
- Feature luks:
  + Add smart card support.
- Feature nfsroot:
  + Fix inconsistency in nfsopts processing.
- Guess subsystem:
  + Guess modules can use infomantion about active feature and re-run guess.
  + Add RECENTLY_ACTIVATED_FEATURE variable to monitor recently added features.
- Misc:
  + create-initrd: Add ability add libraries by name using PUT_FEATURES_LIBS var.
  + Allow to compile and install feature-specific utilities.
  + Add README for all guess modules.
  + Do not fail if autodetection is disabled.

Version 2.19.1

- Feature gpu-drm:
  + Revert "Feature gpu-drm: Check only devices of class PCI_CLASS_DISPLAY_VGA"
  + As a workaround, allow the absence of a module for devices
- New feature:
  + Add new feature sshfsroot. Feature adds the ability to mount the root using
    SSH (more precisely, the SFTP subsystem).

Version 2.19.0

- Feature gpu-drm:
  + Check only devices of class PCI_CLASS_DISPLAY_VGA.
- Utilities:
  + depinfo: Add blacklist support.
- Misc:
  + Show an error if bash not found.
  + Fix feature-info subcommand.
  + Improve documentation.

Version 2.18.0

- Runtime:
  + ueventd: Restore rduevent-timeout=SEC boot parameter.
- New feature:
  + Add new feature add-udev-rules. Udev rules require separate processing
    because other utilities can be called from them.
  + Add new feature gpu-drm. The feature adds to the image the modules needed
    for one or more cards.
- Feature kickstart:
  + Add missing kickstart/data/root directory.
  + Fix fs options that are passed when partitioning disks.
  + Create /run/cryptsetup if it doesn't exist.
- Feature plymouth:
  + Run helpers only once.
  + Fix definition of default theme.
  + Fix plymouth libdir.
- Utilities:
  + initrd-put: Skip the file if it already exists in destdir.
- Misc:
  + Improve documentation.
  + Add Github CI.
  + Run tests for fedora and ubuntu.

Version 2.17.0

- Runtime:
  + Import halt/reboot/poweroff from sysvinit.
  + ueventd: Added the ability to stop processing events in the queue.
  + The stop_daemon should not show stopped pids.
  + Open rdshell by Alt-Uparrow hotkey.
- New feature:
  + kickstart: New feature for automated execution of actions.
- Feature mdadm:
  + Examine only arrays where mountpoints are located.
- Feature luks:
  + Remove only one new line in plain text key mode.
- Feature lkrg:
  + Add nolkrg and noearlylkrg cmdline options (thx Vladimir D. Seleznev).
- Feature plymouth:
  + Improve portability.
  + Run plymouth helpers only if feature is enabled.
- Utilities:
  + depinfo: Check compression suffixes when looking for firmware.
  + depinfo: Explore versioned subdirectories in the firmware search.
- Misc:
  + Rewrite tests.

Version 2.16.0

- Runtime:
  + ueventd tries to process events again if it did not work the first time.
  + Move READONLY handle to fstab service.
  + Fix polld service dependency.
- New feature:
  + iscsi: feature adds you to perform a diskless system boot using pxe and iSCSI (thx Mikhail Chernonog).
- Feature mdadm:
  + md-raid-member handler assume that it has successfully processed all the events.
- Feature pipeline:
  + Use ro,loop options only for a non-device files.

Version 2.15.0

- Runtime:
  + Allow init= to be symlink
  + Fix root=NUMBER
  + Show on console stopped services
  + Make killall messages more informative
- Utilities:
  + initrd-put: Copy absolute symlinks
- Misc:
  + Make a compatibility symlink only if the file doesn't exist
  + Create initramfs filesystem structure based on system filesystem
  + Add more documentation

Version 2.14.0

- Feature mdadm:
  + Generate udev rules for guessed raid devices.
- Feature pipeline:
  + Fix possible race in the waitdev.
- Feature network:
  + Always import runtime environment.
- Runtime:
  + Use wrapper around readlink for portability.
  + Use start-stop-daemon from busybox.
  + Udev variables $ID_\* are optional.
  + Add default udev rules.
  + Add support for root=SERIAL=\*.
- Utilities:
  + initrd-put: Handle symlinks in the root directory.
  + initrd-put: Get the canonical path correctly.
  + initrd-put: Set mode and owner after directories creation.
  + depinfo: Do not show an error if softdep is not found.
- Build:
  + Add busybox and libshell as submodules.
- Misc:
  + All make messages should go to stderr.

Version 2.13.0

- Feature guestfs:
  + Add lable utilities (thx Mikhail Gordeev)
- Feature mdadm:
  + Assemble only $MOUNTPOINTS related raids (thx Slava Aseev)
- Runtime:
  + Support root=PARTLABEL= and root=PARTUUID=
- Utilities:
  + depinfo: Show builtin modules hierarchically if --tree specified.
- Misc:
  + Improve man-pages.
  + Add more tests.

Version 2.12.0

- Feature lkrg:
  + Respect kernel version when we check for a kernel module (thx Vladimir D. Seleznev).
- Misc:
  + initrd-put: Properly handle the situation when the copy_file_range is not
    implemented.

Version 2.11.0

- Feature luks:
  + Decrypt using plymouth if present (thx Oleg Solovyov)
  + Run luks handler after mountdev
- Feature multipath:
  + Add service file and multipathd
- Feature plymouth:
  + Add missing label plugin (thx Oleg Solovyov)
  + Include fonts (thx Oleg Solovyov)
- Runtime:
  + Add rdlog=console boot parameter to send all log messages to the /dev/console
  + Check bootable conditions after each uevend handler
- Misc:
  + Replace initrd-cp by initrd-put

Version 2.10.0

- New feature:
  + Add lkrg feature to preload lkrg module (thx Vladimir D. Seleznev)
- Feature fsck:
  + Show more friendly message
  + Do not check swap
- Feature network:
  + Fix synchronization service
  + Fix generation network config from cmdline
- Feature pipeline:
  + Show an error if the previous step is used which did not exist
  + Run handlers as separate programs
- Feature guestfs:
  + Add file utility (thx Mikhail Gordeev)

Version 2.9.0

- Feature changes:
  + guestfs: Add mke2fs utility
- Utilites:
  + create-initrd: Use bash array to calculate list of files and directories
- Misc:
  + Use bash for scripting
  + Show only actually included features
  + Refactor feature dependencies

Version 2.8.3

- Misc:
  + Guess root: Show device name only
  + tests: Add test for btrfs with subvol

Version 2.8.2

- Feature changes:
  + add-modules: Put MODULES_PRELOAD into the modules-preudev
  + add-modules: Fix kmodule.deps.d execution
- Utilities:
  + bug-report: Fix device list
  + depinfo: Do not stop on error
  + depinfo: Add option to read names from the file
- Misc:
  + Change the priority of directories when copying to an image

Version 2.8.1

- Feature changes:
  + fsck: Always add fsck utilities
- Utilities:
  + make-initrd: Fix --boot=DIR option

Version 2.8.0

- Feature changes:
  + guestfs: Add findfs utility
  + guestfs: Use patterns for utilities
  + guestfs: Add gdisk and sgdisk
  + btrfs: Add all devices in the btrfs
  + network: Add service network-up
- Utilities:
  + Add md_run utility from kinit-utils
  + Add nfsmount utility from kinit-utils
  + Add resume utility from kinit-utils
  + Add runas utility
- Misc:
  + Use autoconf
  + Replace build system
  + Add PUT_FEATURE_PROGS_WILDCARD
  + Refactor features rules
  + Drop bootsplash feature
  + Do not show module dependencies in the guessed config

Version 2.7.0

- New feature:
  + Add sysfs-dma feature to detect dependence on dma by sysfs
  + Add pipeline as an alternative way to search for root
  + Add fsck feature to check filesystem before mount
- Feature changes:
  + virtio-pci: Feature renamed to sysfs-virtio-pci
  + network: Fix cmdline params hack
  + network: preserve iface macaddress
  + nfsroot: Use network feature
- Runtime changes:
  + Export information about configured devices
  + Allow to put the rootdelay on pause
- Misc:
  + Move docs to Documentation
  + Improve documentation
  + Add utility for inspecting bug reports
  + Guess root device based on bug report

Version 2.6.0

- Utilities:
  + make-initrd: Allow to guess modules for any directory
- Misc:
  + Do not use /boot directly
  + Add timestamps to messages
  + Add modules.builtin.modinfo into the initramfs
  + Simplify MOUNTPOINTS processing
  + Allow to use MOUNTPOINTS not only for mount points

Version 2.5.0

- Feature changes:
  + kbd: Reimplement feature
  + mdadm: Try to make problem array writable
  + mdadm: Run mdadm -IRs only once if needed
  + mdadm: Allow to use custom mdadm.conf
  + mdadm: Wait a certain time after the appearance of the raid
    member before starting the degraded raid
  + luks: Allow to skip keydev in the luks-key= and in the /etc/luks.keys
  + usb: Add more usb modules and make them optional
- Misc:
  + guess: Add guessing drm modules
  + Use MODULES_TRY_ADD for hardcoded module lists

Version 2.4.0

- Feature changes:
  + luks: Add essiv for kernel >= 5.4.0
- Runtime changes:
  + Ignore subdirectories in the handlers directory
  + Remove obsolete debug rules
- Utilities:
  + initrd-cp: Use own helper instead of the file utility
  + depinfo: Add modules.builtin.modinfo support
- Misc:
  + Make kernel version check more human readable
  + Add helpers to compare kernel version
  + Add testsuite
  + add-module-pattern: Create tempdir in proper place

Version 2.3.0

- New feature:
  + network: New feature to configure network interfaces in initrd.
- Feature changes:
  + kbd: Configure console fonts if KMS is enabled
  + kbd: Use udev to setup font and keymap
  + kbd: Add guess-script
- Runtime changes:
  + Allow negative values in cmdline parameters
  + Rewrite network configuration
  + Allow to continue boot process after rdshell
  + Re-implement ueventd in shell
  + Allow more than one pre/post script for service
  + Allow run script before and after each service
- Utilities:
  + depinfo: Ignore files in current directory if the argument does
    not look like module name

