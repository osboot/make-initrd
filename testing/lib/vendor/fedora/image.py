# SPDX-License-Identifier: GPL-3.0-or-later

from __future__ import annotations

import textwrap

from lib.image_api import ImageRenderContext, cleanup_command
from lib.template import render_template


def image_cleanup(ctx: ImageRenderContext) -> str:
    return cleanup_command([
        "find /usr/share/locale/ -mindepth 3 -maxdepth 3 -type f -name '*.mo' -delete",
        "find /usr/share/doc/ -mindepth 1 -delete",
        "find /usr/share/groff/ -mindepth 1 -delete",
        "find /usr/share/info/ -mindepth 1 -delete",
        "find /usr/share/licenses/ -mindepth 1 -delete",
        "find /usr/share/man/ -mindepth 1 -delete",
        "find /var/cache/dnf/ -mindepth 1 -delete",
        "find /var/lib/dnf/ -mindepth 1 -delete",
        "find /var/log/anaconda/ -mindepth 1 -delete",
        "find /usr/src/ -mindepth 1 -delete",
        f"find /lib/firmware {ctx.ignore_firmware_expr} -delete",
        f"find /lib/modules {ctx.ignore_kmodules_expr} -delete",
    ])


def render_dockerfiles(ctx: ImageRenderContext) -> tuple[str, str]:
    cleanup = image_cleanup(ctx)
    kernel_packages = ctx.packages.joined_with_arch_suffix("kernel", ctx.arch)
    sys_packages = ctx.packages.joined_with_arch_suffix("sysimage", ctx.arch)
    ks_packages = ctx.packages.joined_with_arch_suffix("kickstart", ctx.arch)

    sysimage = textwrap.dedent(
        f"""\
        FROM {ctx.base_image}
        RUN dnf -y install {kernel_packages}
        RUN dnf -y install {sys_packages}
        RUN dnf -y clean all
        RUN {ctx.depmod_cmd}
        RUN {cleanup}
        RUN {render_template("create-init-script.run.in", {})}
        """
    )
    devel = textwrap.dedent(
        f"""\
        FROM localhost/image-{ctx.vendor_name}:sysimage
        RUN dnf -y install {kernel_packages}
        RUN dnf -y install {ks_packages}
        RUN dnf -y clean all
        RUN {ctx.depmod_cmd}
        RUN {cleanup}
        """
    )
    return sysimage, devel
