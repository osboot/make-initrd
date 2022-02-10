#!/bin/bash -efu

gh_group_start "checking the ability to boot"

for (( i=1; i <= BOOT_DISKS; i++ )); do
	fn="$top_workdir/disk-$i.qcow2"
	qemu_args \
	 -blockdev "driver=qcow2,node-name=disk$i,file.driver=file,file.node-name=file$i,file.filename=$fn" \
	 -device "virtio-blk,drive=disk$i,id=virtio$i"
done > "$top_workdir/qemu-disks"

cat_exec "$top_workdir/qemu.sh" <<-EOF
#!/bin/bash
qemu_args=()
qemu_args+=( -m 1G -no-reboot )
. "$top_workdir/qemu-cpu"
. "$top_workdir/qemu-stdout"
. "$top_workdir/qemu-disks"
set -x
exec qemu-system-$ARCH "\${qemu_args[@]}"
EOF

cat_exec "$top_workdir/run.sh" <<-EOF
#!/bin/bash -x
exec timeout --foreground --signal=TERM --kill-after=5s $TIMEOUT \
	${BOOT_PROG:+"$topdir/testing/$BOOT_PROG"} \
	"$top_workdir/qemu.sh"
EOF

script "$top_logdir/boot.log" -c "$top_workdir/run.sh"

valid_log 'boot check' "$top_logdir/boot.log"

gh_group_end
