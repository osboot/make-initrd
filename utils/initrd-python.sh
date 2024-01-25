#!/bin/bash -efu
# SPDX-License-Identifier: GPL-3.0-or-later

DESTDIR="$1"; shift
PYTHON="$1"; shift

pythonhome="$("$PYTHON" -c 'import inspect, os; print(os.path.dirname(inspect.getabsfile(os)))')"

[ ! -d "$pythonhome" ] ||
	initrd-put "$@" \
		--exclude '.*/(__pycache__|site-packages)(/.*)?$' \
		--exclude '.*\.py[co]$' \
		"$DESTDIR" "$pythonhome"

initrd-put "$@" "$DESTDIR" "$PYTHON"
