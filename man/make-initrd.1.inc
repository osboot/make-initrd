.\" Copyright (C) 2010  Alexey Gladkov <gladkov.alexey@gmail.com>
.\"
.\" Additional documentation for the make-initrd.
.\"
.\" This file is covered by the GNU General Public License,
.\" which should be included with make-initrd as the file LICENSE.

[NAME]
make-initrd \- creates initial ramdisk images for preloading modules

[DESCRIPTION]

[FILES]
.B /etc/initrd.mk
\- Main configuration file for make-initrd;

.B /etc/initrd.mk.d
\- Directory for optional configuration files.

[ENVIRONMENT]
.B INITRD_WORKDIR
\- Sets working directory (default: /tmp or $TMPDIR if set);

.B BLACKLIST_MODULES
\- List of modules that need to be ignored;

.B SYSFS_PATH
\- Sets sysfs placement.
