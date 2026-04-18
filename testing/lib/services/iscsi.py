# SPDX-License-Identifier: GPL-3.0-or-later

from __future__ import annotations

import subprocess

from lib.host_services import (
    HostServiceContext,
    HostServiceError,
    HostServiceProcess,
    wait_log_line,
    wait_tcp_port,
)


PACKAGE_SETS = ("iscsi_service",)

TARGET_NAME = "iqn.2026-04.test.make-initrd:sysimage"
TARGET_PORT = 3260


def configure_initrd(ctx: HostServiceContext, initrd_mk: list[str]) -> None:
    initrd_mk.append("FEATURES += iscsi")


def start(ctx: HostServiceContext) -> HostServiceProcess:
    image = ctx.top_workdir / "sysimage.img"
    if not image.exists():
        raise HostServiceError(f"missing iSCSI backing image: {image}")

    logfile = ctx.top_logdir / "iscsi.log"
    logfile.parent.mkdir(parents=True, exist_ok=True)
    logfh = logfile.open("ab", buffering=0)

    ctx.message(f"starting iscsi service on 0.0.0.0:{TARGET_PORT}")
    proc = subprocess.Popen(
        [
            "podman", "run", "--rm", "--network=host",
            "--volume", f"{image}:/tmp/sysimage.img:rw",
            f"localhost/image-{ctx.vendor_name}:sysimage",
            "sh", "-ec",
            ""
            f"{ctx.install_package_set('iscsi_service')}; "
            "export PATH=/sbin:/usr/sbin:/bin:/usr/bin:$PATH; "
            ""
            "command -v tgtd; "
            "command -v tgtadm; "
            ""
            f"tgtd -f -d 1 --iscsi portal=0.0.0.0:{TARGET_PORT} & "
            "pid=$!; "
            ""
            "trap 'kill \"$pid\" 2>/dev/null || :' EXIT; "
            "for i in $(seq 1 100); do "
            "    tgtadm --lld iscsi --op show --mode target >/dev/null 2>&1 && break; "
            "    sleep 0.2; "
            "done; "
            ""
            "tgtadm --lld iscsi --op show --mode target >/dev/null; "
            f"tgtadm --lld iscsi --op new --mode target --tid 1 -T {TARGET_NAME}; "
            "tgtadm --lld iscsi --op new --mode logicalunit --tid 1 --lun 1 -b /tmp/sysimage.img; "
            "tgtadm --lld iscsi --op bind --mode target --tid 1 -I ALL; "
            "tgtadm --lld iscsi --op show --mode target; "
            ""
            "echo 'ISCSI TARGET READY'; "
            "wait \"$pid\"",
        ],
        cwd=ctx.topdir,
        stdout=logfh,
        stderr=subprocess.STDOUT,
        start_new_session=True,
    )

    if (
        not wait_log_line(logfile, "ISCSI TARGET READY", 180)
        or not wait_tcp_port("127.0.0.1", TARGET_PORT, 10)
    ):
        proc.terminate()
        try:
            proc.wait(timeout=5)
        except subprocess.TimeoutExpired:
            proc.kill()
            proc.wait()
        logfh.close()
        raise HostServiceError(f"iscsi service did not start, see {logfile}")

    return HostServiceProcess(name="iscsi", process=proc, logfh=logfh)
