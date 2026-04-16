# SPDX-License-Identifier: GPL-3.0-or-later

from lib.package_api import PackageSet


def install_command(packages: tuple[str, ...]) -> str:
    return f"dnf -y install {' '.join(packages)}; dnf -y clean all"


def get_packages() -> PackageSet:
    return PackageSet(
        make_initrd=(
            "make", "findutils", "udev", "elfutils-libelf", "kmod-libs", "zlib-ng",
            "bzip2-libs", "libzstd", "xz-libs", "libxcrypt-compat", "json-c",
            "squashfs-tools", "erofs-utils",
        ),
        make_initrd_devel=(
            "make", "findutils", "udev", "elfutils-libelf", "kmod-libs", "zlib-ng",
            "bzip2-libs", "libzstd", "xz-libs", "libxcrypt-compat", "json-c",
            "squashfs-tools", "erofs-utils", "gcc", "make", "automake", "autoconf",
            "bison", "flex", "gawk", "kmod-devel", "zlib-ng-devel", "bzip2-devel",
            "xz-devel", "libxcrypt-devel", "libzstd-devel", "elfutils-libelf-devel",
            "libtirpc-devel", "json-c-devel",
        ),
        kernel=("kernel", "kernel-modules"),
        init=("systemd",),
        sysimage_extra=(
            "pwgen", "mdadm", "lvm2", "cryptsetup", "btrfs-progs", "e2fsprogs",
            "xfsprogs", "dosfstools", "systemd-boot-unsigned", "grub2-pc",
            "grub2-pc-modules",
        ),
        kickstart_extra=(
            "e2fsprogs", "btrfs-progs", "xfsprogs", "dosfstools", "cryptsetup",
            "lvm2", "mdadm", "util-linux", "rsync", "wget", "unzip", "tar",
            "cpio", "pwgen", "zstd", "lz4", "f2fs-tools",
        ),
        sshfs_service=(
            "busybox", "fuse-sshfs", "iproute", "openssh-clients", "openssh-server",
        ),
    )
