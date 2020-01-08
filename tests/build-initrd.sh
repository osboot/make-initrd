#!/bin/sh -efu

read -r testcase < "$HOME/src/.tests/cur"

set -- "$HOME/src/utils/make-initrd" \
	-D \
	-b "$HOME/src/.tests/boot" \
	-c "$HOME/src/tests/$testcase/initrd.mk"

echo "RUN: $*"
"$@"

echo
echo "IT WORKS!"
echo
