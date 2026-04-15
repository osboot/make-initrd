# SPDX-License-Identifier: GPL-3.0-or-later

from lib.package_api import PackageSet


def get_packages() -> PackageSet:
    return PackageSet(
        make_initrd=(
            "app-alternatives/cpio", "app-arch/tar", "dev-build/make",
            "dev-libs/elfutils", "net-libs/libtirpc", "sys-apps/coreutils",
            "sys-apps/shadow", "sys-apps/findutils", "sys-apps/grep", "sys-apps/kmod",
            "sys-apps/util-linux", "virtual/libcrypt:=", "virtual/udev",
            "app-arch/bzip2", "app-arch/xz-utils", "app-text/scdoc", "sys-libs/zlib",
            "app-arch/zstd", "dev-libs/json-c", "app-arch/lz4",
        ),
        make_initrd_devel=(
            "app-alternatives/cpio", "app-arch/tar", "dev-build/make",
            "dev-libs/elfutils", "net-libs/libtirpc", "sys-apps/coreutils",
            "sys-apps/shadow", "sys-apps/findutils", "sys-apps/grep", "sys-apps/kmod",
            "sys-apps/util-linux", "virtual/libcrypt:=", "virtual/udev",
            "app-arch/bzip2", "app-arch/xz-utils", "app-text/scdoc", "sys-libs/zlib",
            "app-arch/zstd", "dev-libs/json-c", "app-arch/lz4", "dev-build/autoconf",
            "dev-build/automake", "dev-util/intltool", "sys-devel/bison",
            "sys-devel/flex", "virtual/pkgconfig",
        ),
        kernel=("sys-kernel/gentoo-kernel-bin",),
        init=("sys-apps/sysvinit",),
        sysimage_extra=(
            "app-admin/pwgen", "sys-fs/mdadm", "sys-fs/lvm2", "sys-fs/cryptsetup",
            "sys-fs/btrfs-progs", "sys-fs/e2fsprogs", "sys-fs/xfsprogs",
            "sys-fs/dosfstools", "sys-boot/grub",
        ),
        kickstart_extra=(
            "sys-fs/e2fsprogs", "sys-fs/btrfs-progs", "sys-fs/xfsprogs",
            "sys-fs/dosfstools", "sys-fs/cryptsetup", "sys-fs/lvm2", "sys-fs/mdadm",
            "sys-apps/util-linux", "net-misc/rsync", "net-misc/wget", "app-arch/unzip",
            "app-arch/tar", "app-arch/cpio", "app-admin/pwgen", "app-arch/zstd",
            "sys-fs/f2fs-tools",
        ),
    )
