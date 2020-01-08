#!/bin/sh -efux

dev="$(blkid -U 72a8b28b-14df-414d-85cd-0359afbd713b)"

expect -f- <<EOF
set timeout 10
spawn cryptsetup open $dev cryptroot
expect "Enter passphrase for $dev: "
send -- "qwerty\r"
expect eof
EOF

mount /dev/mapper/cryptroot "$HOME/rootfs"
