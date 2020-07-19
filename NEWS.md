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

