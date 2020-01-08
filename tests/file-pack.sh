#!/bin/sh -eu

filename="$(readlink -ev "$1")"
outdir="$(readlink -ev "$2")"

name="${filename##*/}"

cd "$outdir"

xz -c < "$filename" |
	split -d -a 6 -b 50M - "$name.xz."

sha256sum "$name.xz."[0-9]* > "$name.SHA256SUMS"
