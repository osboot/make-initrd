#!/bin/sh -efux

umount "$HOME/rootfs"
mdadm --stop /dev/md0
