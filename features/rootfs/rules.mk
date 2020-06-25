rootfs: create
	$Q$(FEATURESDIR)/rootfs/bin/create-fstab

pack: rootfs
