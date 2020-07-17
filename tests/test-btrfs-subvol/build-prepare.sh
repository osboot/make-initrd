#!/bin/sh -efux

modprobe btrfs
btrfs device scan --all-devices

mount -o subvol=@ LABEL=ROOT "$HOME/rootfs"
