#!/bin/bash
# SPDX-License-Identifier: GPL-3.0-or-later

_hook_type="$1"
_action="$2"
sysfs_path="$3"
major="$4"

[ "$major" = 9 ] || [ -d "$sysfs_path/md" ] ||
	exit 2

# Check if array is already inactive or clear
if [ -f "$sysfs_path/md/array_state" ]; then
	state=""

	read -r state < "$sysfs_path/md/array_state" 2>/dev/null ||
		state=""

	[ "$state" != "clear" ] && [ "$state" != "inactive" ] ||
		exit 2
fi

devname="${sysfs_path##*/}"

if command -v mdadm >/dev/null 2>&1; then
	mdadm --stop "/dev/$devname"

elif [ -f "$sysfs_path/md/array_state" ]; then
	echo "idle"     > "$sysfs_path/md/sync_action" 2>/dev/null
	echo "inactive" > "$sysfs_path/md/array_state" 2>/dev/null
	echo "clear"    > "$sysfs_path/md/array_state" 2>/dev/null
else
	exit 1
fi
