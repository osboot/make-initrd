#!/bin/sh -eu

[ -d "$HOME/rootfs" ] ||
	exit 0

cd "$HOME/rootfs"

mkdir -p -- ./bin ./sbin

cp -vf -- /bin/ash.static ./bin/sh

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
