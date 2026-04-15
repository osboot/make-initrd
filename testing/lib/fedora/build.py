# SPDX-License-Identifier: GPL-3.0-or-later

from lib.build_api import BuildRenderContext, KickstartScriptSpec
from lib.build_api import render_build_kickstart_script as render_common_build_kickstart_script
from lib.build_api import render_build_sources_script as render_common_build_sources_script


def render_build_sources_script(ctx: BuildRenderContext) -> str:
    return render_common_build_sources_script(ctx)


def render_build_kickstart_script(ctx: BuildRenderContext) -> str:
    return render_common_build_kickstart_script(ctx, KickstartScriptSpec(
        imagefile="/boot/initrd-ks.img",
        boot_kernel_src="/boot/vmlinuz-$kver",
        boot_initrd_src="/boot/initrd-ks.img",
        copy_kernel_files=True,
        copy_verbose=True,
    ))
