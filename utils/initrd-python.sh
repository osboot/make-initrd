#!/bin/bash -efu

DESTDIR="$1"; shift
PYTHON="$1"; shift

pythonhome="$("$PYTHON" -c 'import inspect, os; print(os.path.dirname(inspect.getabsfile(os)))')"

[ ! -d "$pythonhome" ] ||
	initrd-put "$@" \
		--exclude '.*/(__pycache__|site-packages)(/.*)?$' \
		--exclude '.*\.py[co]$' \
		"$DESTDIR" "$pythonhome"

initrd-put "$@" "$DESTDIR" "$PYTHON"
