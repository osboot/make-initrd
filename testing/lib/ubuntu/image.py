# SPDX-License-Identifier: GPL-3.0-or-later

from __future__ import annotations

import textwrap

from lib.image_api import ImageRenderContext, cleanup_command


APT_ARGS = "-y -qq"


def image_cleanup(ctx: ImageRenderContext) -> str:
    return cleanup_command([
        "find /usr/share/license /usr/share/license-exception -delete",
        "find /usr/share/locale/ -mindepth 3 -maxdepth 3 -type f -name '*.mo' -delete",
        "find /usr/share/doc/ /usr/share/man/ /usr/share/info/ -mindepth 1 -delete",
        "find /var/lib/apt/lists/ /var/cache/apt/ -type f -delete",
        "find /usr/src/ -mindepth 1 -delete",
        f"find /lib/firmware {ctx.ignore_firmware_expr} -delete",
        f"find /lib/modules {ctx.ignore_kmodules_expr} -delete",
    ])


def render_dockerfiles(ctx: ImageRenderContext) -> tuple[str, str]:
    cleanup = image_cleanup(ctx)
    sysimage = textwrap.dedent(
        f"""\
        FROM {ctx.base_image}
        ENV DEBIAN_FRONTEND=noninteractive
        RUN apt-get {APT_ARGS} update
        RUN apt-get {APT_ARGS} install {ctx.packages.joined("kernel")}
        RUN apt-get {APT_ARGS} install {ctx.packages.joined("sysimage")}
        RUN apt-get {APT_ARGS} clean
        RUN {ctx.depmod_cmd}
        RUN {cleanup}
        """
    )
    devel = textwrap.dedent(
        f"""\
        FROM localhost/image-{ctx.vendor_name}:sysimage
        ENV DEBIAN_FRONTEND=noninteractive
        RUN apt-get {APT_ARGS} update
        RUN apt-get {APT_ARGS} install {ctx.packages.joined("kernel")}
        RUN apt-get {APT_ARGS} install {ctx.packages.joined("kickstart")}
        RUN apt-get {APT_ARGS} clean
        RUN {ctx.depmod_cmd}
        RUN {cleanup}
        """
    )
    return sysimage, devel
