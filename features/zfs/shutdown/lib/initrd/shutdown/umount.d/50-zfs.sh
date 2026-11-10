#!/bin/bash
# SPDX-License-Identifier: GPL-3.0-or-later

_hook_type="$1"
_action="$2"
mountpoint="$3"
fstype="$4"

[ "$fstype" = "zfs" ] || exit 2

if command -v zfs >/dev/null 2>&1; then
	zfs unmount -f "$mountpoint" 2>/dev/null ||:
fi

# If all ZFS mountpoints are unmounted, export the pools
if command -v zpool >/dev/null 2>&1; then
	if ! findmnt -rno TARGET -t zfs >/dev/null 2>&1; then
		zpool export -a -f 2>/dev/null || zpool export -a 2>/dev/null ||:
	fi
fi
