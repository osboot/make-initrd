#!/bin/bash -efu

gh_group_start "creating qemu kickstart disks"

for (( i=1; i <= KICKSTART_DISKS; i++ )); do
	message "creating QEMU disk $i ..."

	fn="$top_workdir/disk-$i.qcow2"
	[ -f "$fn" ] ||
		qemu-img create -q -f qcow2 "$fn" 2G

	qemu_args \
	 -blockdev "driver=qcow2,node-name=disk$i,file.driver=file,file.node-name=file$i,file.filename=$fn" \
	 -device "virtio-blk,drive=disk$i,id=virtio$i"
done > "$top_workdir/qemu-disks"

cat_exec "$top_workdir/qemu.sh" <<-EOF
#!/bin/bash
qemu_args=()
qemu_args+=( -m 1G -no-reboot )
qemu_args+=( -kernel "$top_workdir/boot-ks-vmlinuz" )
qemu_args+=( -initrd "$top_workdir/boot-ks-initrd.img" )
qemu_args+=( -append 'console=ttyS0,115200n8 quiet rdlog=console ksfile=ks.cfg' )
. "$top_workdir/qemu-cpu"
. "$top_workdir/qemu-stdout"
. "$top_workdir/qemu-sysimage"
. "$top_workdir/qemu-disks"
set -x
exec qemu-system-$ARCH "\${qemu_args[@]}"
EOF

cat_exec "$top_workdir/run.sh" <<-EOF
#!/bin/bash -x
exec timeout --foreground --signal=TERM --kill-after=5s $TIMEOUT \
	"$top_workdir/qemu.sh"
EOF

script "$top_logdir/ks.log" -c "$top_workdir/run.sh"

valid_log 'kickstart' "$top_logdir/ks.log"

gh_group_end
