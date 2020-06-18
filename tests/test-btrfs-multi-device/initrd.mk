AUTODETECT = all
IMAGEFILE = /root/src/.tests/boot/initramfs-test-btrfs-multi-device.img

MOUNTPOINTS += /root/rootfs
FEATURES += qemu

DISABLE_FEATURES += ucode
DISABLE_GUESS    += ucode
