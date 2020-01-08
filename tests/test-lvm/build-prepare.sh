#!/bin/sh -efux

#/bin/bash

vgchange -v --nolocking --noudevsync --sysinit -ay mygroup
sleep 1

dmsetup mknodes
dev=$(dmsetup info -c -o name --noheadings)

mount "/dev/mapper/$dev" "$HOME/rootfs"
