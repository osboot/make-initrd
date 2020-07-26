#!/bin/sh -eu

read -r testcase < "$HOME/src/.tests/cur"

cp -vf -- /boot/System* "$HOME/src/.tests/boot/"
cp -vf -- /boot/config* "$HOME/src/.tests/boot/"

if [ ! -e "$HOME/src/tests/$testcase/initrd.mk" ]; then
	set -- "$HOME/src/.build/dest/usr/sbin/make-initrd" \
		-D -c /dev/null \
		guess-config

	echo "RUN: $*"
	(
		export MOUNTPOINTS=/root/rootfs
		echo "AUTODETECT ="
		echo "IMAGEFILE = /root/src/.tests/boot/initramfs-$testcase.img"
		echo ""
		"$@"
	) > /tmp/initrd.mk
else
	cp -f -- "$HOME/src/tests/$testcase/initrd.mk" /tmp/initrd.mk
fi

echo ================================================
cat /tmp/initrd.mk
echo ================================================

set -- "$HOME/src/.build/dest/usr/sbin/make-initrd" \
	-D \
	-b "$HOME/src/.tests/boot" \
	-c "/tmp/initrd.mk"

echo "RUN: $*"
"$@"

echo
echo "IT WORKS!"
echo
