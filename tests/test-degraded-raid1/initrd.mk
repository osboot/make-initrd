AUTODETECT = all
IMAGEFILE = /root/src/.tests/boot/initramfs-test-degraded-raid1.img

MOUNTPOINTS += /root/rootfs
FEATURES += qemu

DISABLE_FEATURES += ucode
DISABLE_GUESS    += ucode
