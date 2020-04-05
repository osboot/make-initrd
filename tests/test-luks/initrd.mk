AUTODETECT = all
IMAGEFILE = /root/src/.tests/boot/initramfs-test-luks.img

MOUNTPOINTS = /root/rootfs
FEATURES += qemu

DISABLE_FEATURES += ucode
DISABLE_GUESS    += ucode
