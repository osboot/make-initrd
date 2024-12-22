make-initrd(1)

# NAME

make-initrd - creates initial ramdisk images for preloading modules

# SYNOPSIS

*make-initrd* [<option>]++
*make-initrd* guess-modules [<device>|<directory>]++
*make-initrd* guess-config++
*make-initrd* bug-report

# DESCRIPTION

*make-initrd* is a new, uevent-driven initramfs infrastructure based around udev.
Without options, the program generates images for all the configuration files.

*bug-report*

	Makes the archive for developers when bug happens. The archive contains
	information about disks and partitions and other information about
	hardware. It does not contain information about users.

*guess-config*

	Guesses the current configuration and generates the config file.

*guess-modules* [<device>|<directory>]

	Shows kernel module needed to mount specified mountpoint or block device.

*feature-info* [name [name1...]]

	Shows information about feature. Example:

	```
	$ make-initrd feature-info mdadm luks

	Feature mdadm
	  requires : add-udev-rules
	  disables : raid
	  required : guestfs kickstart

	Feature luks
	  requires : depmod-image devmapper modules-crypto-user-api system-glibc
	  disables :
	  required : guestfs kickstart
	```

*uki*
	Creates an UEFI executable with the kernel, cmdline and initramfs combined.

# OPTIONS
*-D, --no-depmod*
	don’t recreate a list of module dependencies.

*-N, --no-checks*
	don’t check /dev, /proc and the kernel modules directory.

*-c, --config=*_FILE_
	set custom config file.

*-b, --bootdir=*_DIR_
	set boot directory (default: /boot).

*-k, --kernel=*_VERSION_
	set kernel version (default: currently running kernel).

*-v, --verbose*
	print a message for each action.

*-V, --version*
	Show version of program and exit.

*-h, --help*
	Show this text and exit.

# ENVIRONMENT

*INITRD_WORKDIR* - Sets working directory (default: /tmp or $TMPDIR if set).

*BLACKLIST_MODULES* - List of modules that need to be ignored.

*SYSFS_PATH* - Sets sysfs placement.

# FILES

*/etc/initrd.mk* - Main configuration file for make-initrd.

*/etc/initrd.mk.d* - Directory for optional configuration files.

# AUTHOR

Written by Alexey Gladkov, Kirill Shutemov.

# BUGS

Report bugs to the authors.
