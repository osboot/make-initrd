#!/bin/sh -efux

#/bin/bash

mount LABEL=ROOT "$HOME/rootfs"

"$HOME/src/tests/prepare-sysinit.sh"
