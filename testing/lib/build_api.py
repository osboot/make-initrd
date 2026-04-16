# SPDX-License-Identifier: GPL-3.0-or-later

from __future__ import annotations

from dataclasses import dataclass
import shlex
import textwrap


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


def container_workdir(ctx: BuildRenderContext) -> str:
    return f"{ctx.builddir}/{ctx.workdir_rel}"


def render_build_sources_script(ctx: BuildRenderContext, *, pre_configure: str = "") -> str:
    return textwrap.dedent(
        f"""\
        #!/bin/bash -efux

        cd {shlex.quote(ctx.builddir)}
        rc=0

        {pre_configure}
        ./autogen.sh
        ./configure \\
            --enable-local-build \\
            --with-runtimedir=/lib/initrd1 || rc=$?

        if [ "$rc" != 0 ]; then
            echo '========================================='
            cat config.log
            exit 1
        fi

        make || rc=$?

        if [ "$rc" != 0 ]; then
            echo '========================================='
            cat config.log
            exit 1
        fi
        """
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

    return textwrap.dedent(
        f"""\
        #!/bin/bash -efux

        kver="$(find /lib/modules -mindepth 1 -maxdepth 1 -printf '%f\\n' -quit)"

        cat > /etc/initrd.mk <<'EOF1'
        {initrd_mk}
        EOF1

        export PATH={shlex.quote(ctx.builddir)}/.build/dest/usr/sbin:{shlex.quote(ctx.builddir)}/.build/dest/usr/bin:$PATH

        {shlex.quote(ctx.builddir)}/.build/dest/usr/sbin/make-initrd -vv -k "$kver"

        cp -vL {spec.boot_kernel_src} {outdir}/boot-ks-vmlinuz
        cp -vL {spec.boot_initrd_src} {outdir}/boot-ks-initrd.img
        """
    )
