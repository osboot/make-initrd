#!/bin/bash
# SPDX-License-Identifier: GPL-3.0-or-later

_hook_type="$1"
_action="$2"
mountpoint="$3"
fstype="$4"

[ "$fstype" = "btrfs" ] || exit 2

if command -v btrfs >/dev/null 2>&1; then
	btrfs filesystem sync "$mountpoint" 2>/dev/null ||:
fi
