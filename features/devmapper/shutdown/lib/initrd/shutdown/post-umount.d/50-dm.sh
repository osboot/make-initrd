#!/bin/bash
# SPDX-License-Identifier: GPL-3.0-or-later

_hook_type="$1"
_action="$2"
sysfs_path="$3"
_major="$4"

[ -d "$sysfs_path/dm" ] ||
	exit 2

command -v dmsetup >/dev/null 2>&1 ||
	exit 1

read -r dm_name < "$sysfs_path/dm/name" 2>/dev/null ||
	exit 1

[ -n "$dm_name" ] ||
	exit 1

dmsetup remove --retry "$dm_name"
