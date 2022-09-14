#!/bin/bash -efu

gh_group_start "packing sysimage"

cat_exec "$top_workdir/run.sh" <<-EOF
#!/bin/bash -efux

bytes=\$(du -sb /image |cut -f1)
bytes=\$(( \$bytes + ( ( \$bytes * 30 ) / 100 ) ))

dd count=1 bs=\$bytes if=/dev/zero of=/host/$workdir/sysimage.img
/sbin/mke2fs -t ext3 -L SYSIMAGE -d /image /host/$workdir/sysimage.img
EOF

run podman run --rm -ti \
	--mount="type=image,src=localhost/mi-$VENDOR:sysimage,dst=/image" \
	--mount="type=tmpfs,destination=/image$builddir" \
	--mount="type=bind,src=$topdir/.build/dest,dst=/image$builddir/.build/dest" \
	--mount="type=bind,src=$topdir/data,dst=/image$builddir/data" \
	--mount="type=bind,src=$topdir/features,dst=/image$builddir/features" \
	--mount="type=bind,src=$topdir/guess,dst=/image$builddir/guess" \
	--mount="type=bind,src=$topdir/tools,dst=/image$builddir/tools" \
	--volume="$topdir:/host" \
	"localhost/mi-$VENDOR:sysimage" "/host/$workdir/run.sh"

qemu_args \
 -blockdev "driver=file,node-name=sysimage,filename=$top_workdir/sysimage.img" \
 -device "virtio-blk,drive=sysimage" \
> "$top_workdir/qemu-sysimage"

gh_group_end
