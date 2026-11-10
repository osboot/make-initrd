make-initrd-shutdown-restore(1)

# NAME

make-initrd-shutdown-restore - restores initramfs for clean shutdown (exitrd)

# SYNOPSIS

*make-initrd-shutdown-restore*

# DESCRIPTION

*make-initrd-shutdown-restore* is a system shutdown utility that prepares the
environment for *systemd*(1) to pivot back into an exitrd during system shutdown
or reboot.

Before local filesystems are unmounted during shutdown, the utility locates the
active kernel's initramfs using *initrd-find*(1) and inspects its contents using
*initrd-ls*(1). If the image contains the *\/shutdown* executable (meaning that
the *shutdown* feature was included in the initramfs), the archive is unpacked
into the target directory (*\/run/initramfs* by default) using *initrd-extract*(1).
If compressed filesystem layers are present (such as SquashFS or EROFS), they are
unpacked as well.

If *\/run/initramfs/shutdown* already exists and is executable, the utility does
nothing and exits immediately with status 0. If the found initramfs does not
contain the *shutdown* feature, the utility exits cleanly without error, allowing
distributions to enable *make-initrd-shutdown.service* unconditionally.

During the final shutdown phase, *systemd-shutdown*(8) pivots root to
*\/run/initramfs* and executes *\/shutdown*, which cleanly terminates remaining
processes, unmounts filesystems, deactivates device-mapper and RAID block devices,
and halts or reboots the machine.

# ENVIRONMENT

*TARGET_DIR*
	Destination directory where the initramfs should be unpacked.
	Defaults to *\/run/initramfs*.

*SYS_KVER*
	Kernel version to search an initramfs for. Defaults to the running
	kernel version (_uname -r_).

*BOOT_DIRS*
	Whitespace-separated list of candidate boot directories passed to
	*initrd-find*(1).

*VERBOSE*
	If set to a non-empty value, enables diagnostic output to standard error.

# EXIT STATUS

*0*
	Initramfs was successfully unpacked, *\/shutdown* was already present, or
	the initramfs does not contain the *shutdown* feature.

*1*
	Required utilities (*initrd-find*, *initrd-extract*, *initrd-ls*) are missing,
	or *\/shutdown* was not found after unpacking.

# AUTHOR

Written by Alexey Gladkov <gladkov.alexey@gmail.com>.

# BUGS

Report bugs to the authors.

# SEE ALSO

*initrd-find*(1), *make-initrd*(1), *systemd-shutdown*(8)
