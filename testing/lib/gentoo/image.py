# SPDX-License-Identifier: GPL-3.0-or-later

from __future__ import annotations

import textwrap

from lib.image_api import ImageRenderContext, cleanup_command


def image_cleanup(ctx: ImageRenderContext) -> str:
    return cleanup_command([
        "find /usr/share/locale/ -mindepth 3 -maxdepth 3 -type f -name '*.mo' -delete",
        "find /usr/share/doc/ -mindepth 1 -delete",
        "find /usr/share/groff/ -mindepth 1 -delete",
        "find /usr/share/info/ -mindepth 1 -delete",
        "find /usr/share/man/ -mindepth 1 -delete",
        "find /var/tmp/portage/ -mindepth 1 -delete",
        "find /usr/src/ -mindepth 1 -delete",
        f"find /lib/firmware {ctx.ignore_firmware_expr} -delete",
        f"find /lib/modules {ctx.ignore_kmodules_expr} -delete",
    ])


def render_dockerfiles(ctx: ImageRenderContext) -> tuple[str, str]:
    cleanup = image_cleanup(ctx)
    emerge_args = f"--quiet --ask=n --getbinpkg --autounmask-write --jobs={ctx.cpu_count} --newuse --update"
    sysimage = textwrap.dedent(
        f"""\
        FROM {ctx.base_image}
        ENV FEATURES="-ipc-sandbox -network-sandbox -pid-sandbox"
        ENV USE="boot lvm -initramfs"
        ENV ACCEPT_KEYWORDS="~amd64"
        RUN getuto >/dev/null
        RUN emerge-webrsync --quiet
        RUN emerge {emerge_args} {ctx.packages.joined("kernel")}
        RUN emerge {emerge_args} {ctx.packages.joined("sysimage")}
        RUN emerge --depclean --with-bdeps=n --ask=n
        RUN {ctx.depmod_cmd}
        RUN find /lib/modules -mindepth 1 -maxdepth 1 -printf '%f\\n' | while read -r kver; do for i in vmlinuz System.map config; do if [ -f "/lib/modules/$kver/$i" ]; then cp -vL "/lib/modules/$kver/$i" "/lib/modules/$kver/$i.real"; rm -v "/lib/modules/$kver/$i"; mv -v "/lib/modules/$kver/$i.real" "/lib/modules/$kver/$i"; fi; done; done;
        RUN {cleanup}
        RUN for i in $(portageq pkgdir) $(portageq distdir) $(portageq get_repo_path / gentoo); do find "$i" -mindepth 1 -delete; done
        RUN mkdir -p /srv
        """
    )
    devel = textwrap.dedent(
        f"""\
        FROM localhost/image-{ctx.vendor_name}:sysimage
        ENV USE="boot lvm -initramfs"
        RUN emerge-webrsync --quiet
        RUN emerge {emerge_args} {ctx.packages.joined("kernel")}
        RUN emerge {emerge_args} {ctx.packages.joined("kickstart")}
        RUN emerge --depclean --with-bdeps=n --ask=n
        RUN {ctx.depmod_cmd}
        RUN {cleanup}
        RUN mkdir -p /srv
        """
    )
    return sysimage, devel
