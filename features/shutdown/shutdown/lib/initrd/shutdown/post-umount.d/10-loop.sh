#!/bin/bash
# SPDX-License-Identifier: GPL-3.0-or-later

_hook_type="$1"
_action="$2"
sysfs_path="$3"
major="$4"

[ "$major" = 7 ] || [ -d "$sysfs_path/loop" ] ||
	exit 2

command -v losetup >/dev/null 2>&1 ||
	exit 1

# If loop device has no backing file, nothing to detach
[ -s "$sysfs_path/loop/backing_file" ] ||
	exit 2

devname="${sysfs_path##*/}"

losetup -d "/dev/$devname"
