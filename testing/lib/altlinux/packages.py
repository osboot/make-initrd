# SPDX-License-Identifier: GPL-3.0-or-later

from lib.package_api import PackageSet


APT_ARGS = "-y -qq -o=APT::Install::Virtual=true -o=APT::Install::VirtualVersion=true"


def install_command(packages: tuple[str, ...]) -> str:
    return f"apt-get {APT_ARGS} update; apt-get {APT_ARGS} install {' '.join(packages)}; apt-get {APT_ARGS} clean"


def get_packages() -> PackageSet:
    return PackageSet(
        make_initrd=(
            "make", "udev", "libelf", "libkmod", "zlib", "bzlib", "libzstd",
            "liblzma", "libbpf", "libjson-c5", "squashfs-tools",
        ),
        make_initrd_devel=(
            "make", "udev", "libelf", "libkmod", "zlib", "bzlib", "libzstd",
            "liblzma", "libbpf", "libjson-c5", "squashfs-tools", "gcc", "make",
            "automake", "autoconf", "bison", "flex", "libkmod-devel", "zlib-devel",
            "bzlib-devel", "liblzma-devel", "libzstd-devel", "libelf-devel",
            "libtirpc-devel", "libcrypt-devel", "libjson-c-devel",
        ),
        kernel=("kernel-latest",),
        init=("systemd",),
        sysimage_extra=(
            "pwgen", "mdadm", "lvm2", "cryptsetup", "btrfs-progs", "e2fsprogs",
            "xfsprogs", "dosfstools", "reiserfsprogs", "grub-common", "grub-efi",
            "grub-pc",
        ),
        kickstart_extra=(
            "util-linux", "rsync", "wget", "cryptsetup", "lvm2", "mdadm", "pwgen",
            "e2fsprogs", "btrfs-progs", "xfsprogs", "dosfstools", "reiserfsprogs",
            "sfdisk", "unzip", "tar", "cpio", "eject", "lz4", "f2fs-tools",
        ),
        sshfs_service=(
            "busybox", "fuse-sshfs", "iproute2", "openssh-clients", "openssh-server",
        ),
        iscsi_service=(
            "busybox", "iproute2", "open-iscsi", "scsitarget-utils",
        ),
    )
