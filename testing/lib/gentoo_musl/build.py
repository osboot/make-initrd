# SPDX-License-Identifier: GPL-3.0-or-later

import textwrap

from lib.build_api import BootScriptSpec, BuildRenderContext
from lib.build_api import render_build_kickstart_script as render_common_build_kickstart_script
from lib.build_api import render_build_sources_script as render_common_build_sources_script


def render_build_sources_script(ctx: BuildRenderContext) -> str:
    pre_configure = textwrap.dedent(
        """\
        # Some busybox features are broken with musl.
        # See https://wiki.musl-libc.org/building-busybox
        sed -e 's/^CONFIG_FEATURE_VI_REGEX_SEARCH=y/# CONFIG_FEATURE_VI_REGEX_SEARCH is not set/' \\
            -i external/busybox/config
        """
    )
    return render_common_build_sources_script(ctx, pre_configure=pre_configure)


def boot_script_spec() -> BootScriptSpec:
    return BootScriptSpec(
        imagefile="/boot/initrd-ks.img",
        boot_kernel_src="/lib/modules/$kver/vmlinuz",
        boot_initrd_src="/boot/initrd-ks.img",
        install_services=False,
    )


def render_build_kickstart_script(ctx: BuildRenderContext) -> str:
    return render_common_build_kickstart_script(ctx, boot_script_spec())
