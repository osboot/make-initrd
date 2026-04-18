# SPDX-License-Identifier: GPL-3.0-or-later

from __future__ import annotations

from dataclasses import dataclass
import shlex

from lib.template import render_template


@dataclass(frozen=True)
class BuildRenderContext:
    builddir: str
    workdir_rel: str
    top_workdir: str


@dataclass(frozen=True)
class BootScriptSpec:
    imagefile: str | None
    boot_kernel_src: str
    boot_initrd_src: str
    install_services: bool


def container_workdir(ctx: BuildRenderContext) -> str:
    return f"{ctx.builddir}/{ctx.workdir_rel}"


def render_build_sources_script(ctx: BuildRenderContext, *, pre_configure: str = "") -> str:
    return render_template(
        "build-sources.sh.in",
        {
            "BUILDDIR": shlex.quote(ctx.builddir),
            "PRE_CONFIGURE": pre_configure,
        },
    )


def render_build_kickstart_script(ctx: BuildRenderContext, spec: BootScriptSpec) -> str:
    outdir = shlex.quote(container_workdir(ctx))
    common = [" AUTODETECT ="]
    if spec.imagefile is not None:
        common.append(f" IMAGEFILE = {spec.imagefile}")
    common.extend([
        " DISABLE_FEATURES += ucode",
        " FEATURES = add-modules add-udev-rules kickstart qemu rdshell modules-virtio modules-blockdev",
        " MODULES_ADD += sd_mod",
        " MODULES_PRELOAD += virtio-blk virtio-scsi virtio-pci",
        f" KICKSTART_CONFIGS = {ctx.builddir}/{ctx.workdir_rel}/ks.cfg",
        " PUT_PROGS += pwgen",
    ])
    initrd_mk = "\n".join(common)

    return render_template(
        "build-kickstart.sh.in",
        {
            "BUILDDIR": shlex.quote(ctx.builddir),
            "INITRD_MK": initrd_mk,
            "BOOT_KERNEL_SRC": spec.boot_kernel_src,
            "BOOT_INITRD_SRC": spec.boot_initrd_src,
            "OUTDIR": outdir,
        },
    )
