initrd-find(1)

# NAME

initrd-find - finds matching initramfs or Unified Kernel Image (UKI) for a kernel

# SYNOPSIS

*initrd-find* [options] [KERNEL_VERSION]

# DESCRIPTION

*initrd-find* locates the initial ramdisk (initramfs) or Unified Kernel Image (UKI)
matching a specified kernel version. If no kernel version is specified, the
running kernel version (_uname -r_) is used.

The utility searches the following locations in order:

. Boot Loader Specification (BLS) Type #1 entry files (*loader/entries/\*.conf*) in candidate boot and ESP directories. Multi-part initrd directives are supported.
. BLS Type #1 directory hierarchies (*$BOOT/$TOKEN/$KERNEL_VERSION/initrd\**).
. BLS Type #2 Unified Kernel Images (*$BOOT/EFI/Linux/\*.efi*).
. Kernel command line parameters (*initrd=...*) from */proc/cmdline*.
. Distro module directory (*\/lib/modules/$KERNEL_VERSION/initrd*).
. Traditional boot directory naming schemes (*initrd-$KERNEL_VERSION.img*, *initramfs-$KERNEL_VERSION.img*, etc.) in candidate boot directories.
. Root directory fallback (*/initrd.img*).

By default, candidate images are verified to be created by *make-initrd* by checking
for the identification signature *ID=make-initrd* in *etc/initrd-release*. Use
*-a, --any* to accept any initramfs image.

Upon success, *initrd-find* prints the path of the found image to standard output
and exits with status 0. If no matching image is found, it exits with status 1.

# OPTIONS

*-k, --kernel*=_VERSION_
	Specifies the kernel version to find an initramfs for. Defaults to the running
	kernel version.

*-b, --boot-dir*=_DIR_
	Adds a directory to the list of candidate boot directories to search for
	boot loader entries and images. This option can be specified multiple times.

*-r, --root-dir*=_DIR_
	Specifies the root directory under which to search for boot directories
	and images (default: _/_). If specified, default boot paths and relative
	or absolute boot directories given to *-b* are resolved within this root.

*-m, --mount*
	Mounts candidate boot and ESP partitions (*\/boot*, *\/efi*, *\/boot/efi*, and
	paths reported by *bootctl*(1)) read-only if they are not already mounted.
	Temporary mounts performed by the tool are automatically unmounted upon exit.

*--no-mount*
	Do not attempt to mount filesystems (default).

*-a, --any*
	Accept any matching initramfs image, even if it does not contain the *make-initrd*
	signature in *etc/initrd-release*.

*-v, --verbose*
	Print diagnostic information about the search process to standard error.

*-q, --quiet*
	Suppress warning messages when no image is found.

*-V, --version*
	Print program version and exit.

*-h, --help*
	Show help message and exit.

# ENVIRONMENT

*ROOT_DIR*
	Root directory to search under if *-r* is not specified.

*SYS_KVER*
	Kernel version to search for if *-k* is not specified.

*BOOT_DIRS*
	Whitespace-separated list of candidate boot directories to search.

# EXIT STATUS

*0*
	Matching initramfs or UKI image was found.

*1*
	No matching image was found or invalid options were given.

# AUTHOR

Written by Alexey Gladkov <gladkov.alexey@gmail.com>.

# BUGS

Report bugs to the authors.

# SEE ALSO

*make-initrd*(1), *initrd-ls*(1), *initrd-extract*(1), *make-initrd-shutdown-restore*(1)
