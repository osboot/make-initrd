#!/bin/sh -efux

modprobe btrfs
btrfs device scan --all-devices

mount LABEL=ROOT "$HOME/rootfs"
