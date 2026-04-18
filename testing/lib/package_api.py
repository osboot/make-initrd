# SPDX-License-Identifier: GPL-3.0-or-later

from __future__ import annotations

from dataclasses import dataclass, field


SERVICE_PACKAGE_SETS = (
    "sshfs_service",
    "iscsi_service",
    "nvmf_service",
)


@dataclass(frozen=True)
class PackageSet:
    make_initrd: tuple[str, ...]
    make_initrd_devel: tuple[str, ...]
    kernel: tuple[str, ...]
    init: tuple[str, ...]
    sysimage_extra: tuple[str, ...]
    kickstart_extra: tuple[str, ...]
    sshfs_service: tuple[str, ...] = field(default_factory=tuple)
    iscsi_service: tuple[str, ...] = field(default_factory=tuple)
    nvmf_service: tuple[str, ...] = field(default_factory=tuple)
    zfs: tuple[str, ...] = field(default_factory=tuple)

    def packages(self, name: str) -> tuple[str, ...]:
        if name == "make_initrd":
            return self.make_initrd
        if name == "make_initrd_devel":
            return self.make_initrd_devel
        if name == "kernel":
            return self.kernel
        if name == "init":
            return self.init
        if name == "zfs":
            return self.zfs
        if name in SERVICE_PACKAGE_SETS:
            return getattr(self, name)
        if name == "services":
            packages: tuple[str, ...] = ()
            for package_set in SERVICE_PACKAGE_SETS:
                packages += self.packages(package_set)
            return packages
        if name == "sysimage":
            return self.make_initrd + self.init + self.sysimage_extra + self.zfs
        if name == "kickstart":
            return self.make_initrd_devel + self.init + self.kickstart_extra + self.zfs
        raise KeyError(name)

    def joined(self, name: str) -> str:
        return " ".join(self.packages(name))

    def joined_with_arch_suffix(self, name: str, arch: str) -> str:
        result: list[str] = []
        for package in self.packages(name):
            if package.startswith("lib") or package.endswith("-devel"):
                result.append(f"{package}.{arch}")
            else:
                result.append(package)
        return " ".join(result)
