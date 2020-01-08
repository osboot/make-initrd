#!/bin/sh -efux

umount "$HOME/rootfs"
vgchange -f --nolocking -an mygroup
