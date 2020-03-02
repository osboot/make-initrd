#!/bin/sh -efux

read -r testcase < "$HOME/src/.tests/cur" ||:

mdadm --assemble \
	-c "$HOME/src/tests/$testcase/mdadm.conf" \
	/dev/md0 /dev/sdb /dev/sdc

mount /dev/md0 "$HOME/rootfs"
