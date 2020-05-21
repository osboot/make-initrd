AUTODETECT = all
IMAGEFILE = /root/src/.tests/boot/initramfs-test-pipeline-iso-squash.img

MOUNTPOINTS += /root/rootfs
FEATURES += qemu pipeline

DISABLE_FEATURES += ucode
DISABLE_GUESS    += ucode
