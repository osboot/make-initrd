#!/bin/sh -efu

#exec /bin/bash

echo
echo "Cleanup make-initrd ..."
echo
make -C "$HOME/src" clean

echo
echo "Building make-initrd ..."
echo
make -C "$HOME/src" MKLOCAL=1

echo
echo "IT WORKS!"
echo
