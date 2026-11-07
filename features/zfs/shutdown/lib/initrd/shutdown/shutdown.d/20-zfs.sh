#!/bin/bash
# SPDX-License-Identifier: GPL-3.0-or-later

_hook_type="$1"
_action="$2"

if command -v zpool >/dev/null 2>&1; then
	zpool export -a -f 2>/dev/null || zpool export -a 2>/dev/null ||:
fi
