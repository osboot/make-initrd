#!/bin/sh -eu

[ -d "$HOME/rootfs" ] ||
	exit 0

cd "$HOME/rootfs"

mkdir -p -- ./bin ./sbin

~/src/.build/dest/usr/bin/initrd-put . /bin/bash

cat > ./sbin/init <<EOF
#!/bin/sh

echo
echo
echo 'IT WORKS!'
echo
echo

for i in 1 2 3 4 5 6 7 8 9; do
	echo
done
EOF
chmod +x ./sbin/init
