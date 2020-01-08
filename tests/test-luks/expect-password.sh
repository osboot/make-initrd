#!/bin/sh -efu

prog="$(readlink -ev "$0")"
prog="${prog##*/}"
TMPDIR="${TMPDIR:-/tmp}"

{
	printf '#!/bin/sh\n'
	printf 'exec'
	printf " '%s'" "$@"
	printf '\n'
} >"$TMPDIR/$prog"
chmod +x "$TMPDIR/$prog"

expect -f- <<EOF
set timeout -1
spawn "$TMPDIR/$prog"
set qemu_id \$spawn_id
expect -i \$qemu_id "Enter passphrase for /dev/sda2: "
send -i \$qemu_id "qwerty\r"
expect eof
EOF
