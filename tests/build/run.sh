#!/bin/sh -efu

#exec /bin/bash

export MKLOCAL=1

echo
echo "Cleanup make-initrd ..."
echo
make -C "$HOME/src" clean

echo
echo "Building make-initrd ..."
echo
make -C "$HOME/src"

echo
echo "IT WORKS!"
echo
