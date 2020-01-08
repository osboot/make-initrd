#!/bin/sh -efux

umount "$HOME/rootfs"
cryptsetup close cryptroot
