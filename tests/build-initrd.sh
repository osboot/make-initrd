#!/bin/sh -eu

read -r testcase < "$HOME/src/.tests/cur"

cp -vf -- /boot/System* "$HOME/src/.tests/boot/"
cp -vf -- /boot/config* "$HOME/src/.tests/boot/"

set -- "$HOME/src/utils/make-initrd" \
	-D \
	-b "$HOME/src/.tests/boot" \
	-c "$HOME/src/tests/$testcase/initrd.mk"

echo "RUN: $*"
"$@"

echo
echo "IT WORKS!"
echo
