#!/bin/sh -efu

export LANG=C

url="$1"; shift
checksums="$1"; shift
outdir="$(readlink -ev "$1")"; shift

cd "$outdir"

curl -sL -o "${checksums##*/}" "$url/${checksums##*/}"
checksums="$outdir/${checksums##*/}"

! sha256sum -c "$checksums" >/dev/null 2>&1 ||
	exit 0

name=
while read -r chksum filename; do
	name="$filename"
	curl -sL -o "$filename" "$url/$filename"
done < "$checksums"
name=${name%.[0-9]*}

sha256sum -c "$checksums"

case "$name" in
	*.xz) compressor='xz -d -c'   ;;
	*.gz) compressor='gzip -d -c' ;;
	*)    compressor='cat'        ;;
esac

while read -r chksum filename; do
	cat "$filename"
done < "$checksums" |
	$compressor > "${name%.*}"

echo "${name%.*}: DONE"
echo
