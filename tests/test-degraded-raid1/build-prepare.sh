#!/bin/sh -efux

read -r testcase < "$HOME/src/.tests/cur" ||:

disk1="$(blkid --output 'device' --match-token UUID_SUB="82dd176e-ade8-1600-194d-7f7d36f22fb8")"
disk2="$(blkid --output 'device' --match-token UUID_SUB="843dc522-ab96-32bb-8143-b02904fa2b2f")"

mdadm --assemble \
	-c "$HOME/src/tests/$testcase/mdadm.conf" \
	/dev/md0 "$disk1" "$disk2"

mount /dev/md0 "$HOME/rootfs"
