# SPDX-License-Identifier: GPL-3.0-or-later

from __future__ import annotations

from dataclasses import dataclass

from lib.package_api import PackageSet


@dataclass(frozen=True)
class ImageRenderContext:
    vendor_name: str
    base_image: str
    arch: str
    packages: PackageSet
    depmod_cmd: str
    ignore_firmware_expr: str
    ignore_kmodules_expr: str
    cpu_count: int


def cleanup_command(parts: list[str]) -> str:
    return " ||:; ".join(parts) + " ||:;"


def create_init_script() -> str:
    return """\
        printf > /init.once '%s\\n' \\
            '#!/bin/sh' \\
            'test -d /proc && mount -t proc proc /proc;' \\
            'test -d /sys && mount -t sysfs sysfs /sys;' \\
            'echo; echo; echo; echo "IT WORKS!"; echo; echo; echo;' \\
            'sync; reboot -f || poweroff -f || halt -f' \\
            'exit 0'; \\
        chmod 755 /init.once;
        """
