# SPDX-License-Identifier: GPL-3.0-or-later

from lib.build_api import BootScriptSpec, BuildRenderContext
from lib.build_api import render_build_kickstart_script as render_common_build_kickstart_script
from lib.build_api import render_build_sources_script as render_common_build_sources_script


def render_build_sources_script(ctx: BuildRenderContext) -> str:
    return render_common_build_sources_script(ctx)


def boot_script_spec() -> BootScriptSpec:
    return BootScriptSpec(
        imagefile="/boot/initrd-ks.img",
        boot_kernel_src="/boot/vmlinuz",
        boot_initrd_src="/boot/initrd-ks.img",
        install_services=True,
    )


def render_build_kickstart_script(ctx: BuildRenderContext) -> str:
    return render_common_build_kickstart_script(ctx, boot_script_spec())
