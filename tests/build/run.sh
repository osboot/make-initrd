#!/bin/sh -efu

#exec /bin/bash

cd "$HOME/src"

echo "RUN: ./autogen.sh"
./autogen.sh

echo "RUN: ./configure --enable-local-build"
./configure --enable-local-build

echo "RUN: make clean"
make clean

echo "RUN: make"
make

echo "LIST"
find .build/dest -type f -printf '%m %p\n'

echo
echo "IT WORKS!"
echo
