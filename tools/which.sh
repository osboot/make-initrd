#!/bin/sh -efu

name="${1?Name required}"
path="$PATH"

while [ -n "$path" ]; do
	d="${path%%:*}"
	if [ -n "$d" ]; then
		x="${d%/}/$name"
		if [ -e "$x" ]; then
			printf '%s\n' "$x"
			exit 0
		fi
	fi
	path="${path#$d}"
	path="${path#:}"
done
printf >&2 'which.sh: no %s in (%s)\n' "$name" "$PATH"
exit 1
