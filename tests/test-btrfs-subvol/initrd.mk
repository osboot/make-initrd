AUTODETECT = all
IMAGEFILE = /root/src/.tests/boot/initramfs-test-btrfs-subvol.img

MOUNTPOINTS += /root/rootfs
FEATURES += qemu

DISABLE_FEATURES += ucode
DISABLE_GUESS    += ucode
