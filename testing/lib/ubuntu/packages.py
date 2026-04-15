# SPDX-License-Identifier: GPL-3.0-or-later

from lib.package_api import PackageSet


def get_packages() -> PackageSet:
    return PackageSet(
        make_initrd=(
            "make", "udev", "libelf1", "libkmod2", "zlib1g", "libbz2-1.0",
            "libzstd1", "liblzma5", "libjson-c5", "squashfs-tools", "erofs-utils",
        ),
        make_initrd_devel=(
            "make", "udev", "libelf1", "libkmod2", "zlib1g", "libbz2-1.0",
            "libzstd1", "liblzma5", "libjson-c5", "squashfs-tools", "erofs-utils",
            "bison", "flex", "bzip2", "gcc", "make", "automake", "autoconf",
            "pkg-config", "udev", "scdoc", "libkmod-dev", "libz-dev", "libbz2-dev",
            "liblzma-dev", "libzstd-dev", "libelf-dev", "libtirpc-dev",
            "libcrypt-dev", "libjson-c-dev",
        ),
        kernel=("linux-generic",),
        init=("systemd",),
        sysimage_extra=(
            "pwgen", "mdadm", "lvm2", "cryptsetup", "btrfs-progs", "e2fsprogs",
            "xfsprogs", "dosfstools", "systemd-boot", "systemd-boot-efi", "grub-pc",
        ),
        kickstart_extra=(
            "e2fsprogs", "btrfs-progs", "xfsprogs", "dosfstools", "cryptsetup",
            "lvm2", "mdadm", "util-linux", "rsync", "wget", "unzip", "tar",
            "cpio", "pwgen", "zstd", "lz4", "eject", "fdisk", "f2fs-tools",
        ),
        zfs=("zfsutils-linux",),
    )
