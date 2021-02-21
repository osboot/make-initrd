#!/bin/sh -efux

read -r testcase < "$HOME/src/.tests/cur" ||:

mdadm --assemble --scan \
	-c "$HOME/src/tests/$testcase/mdadm.conf"

mount LABEL=ROOT "$HOME/rootfs"
