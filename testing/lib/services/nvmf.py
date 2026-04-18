# SPDX-License-Identifier: GPL-3.0-or-later

from __future__ import annotations

import shlex
import subprocess

from lib.host_services import HostServiceContext, HostServiceError, HostServiceProcess, wait_tcp_port


PACKAGE_SETS = ("nvmf_service",)

TARGET_NAME = "nqn.2026-04.test.make-initrd:sysimage"
TARGET_PORT = 4420
HOST_NQN = "nqn.2026-04.test.make-initrd:host"
HOST_ID = "11111111-2222-4333-8444-555555555555"


def configure_initrd(ctx: HostServiceContext, initrd_mk: list[str]) -> None:
    initrd_mk.append("FEATURES += nvmf")


def _run_script(script: str, logfh) -> None:
    subprocess.run(
        ["sudo", "sh", "-ec", script],
        stdout=logfh,
        stderr=subprocess.STDOUT,
        check=True,
    )


def _run_cleanup(script: str, logfh) -> None:
    subprocess.run(
        ["sudo", "sh", "-ec", script],
        stdout=logfh,
        stderr=subprocess.STDOUT,
        check=False,
    )


def _cleanup_script() -> str:
    return f"""\
base=/sys/kernel/config/nvmet
subsys="$base/subsystems/{TARGET_NAME}"
port="$base/ports/1"
ns="$subsys/namespaces/1"

[ ! -L "$port/subsystems/{TARGET_NAME}" ] || rm -f "$port/subsystems/{TARGET_NAME}"
[ ! -e "$ns/enable" ] || echo 0 > "$ns/enable"
rmdir "$ns" "$subsys/namespaces" "$subsys" "$port/subsystems" "$port" 2>/dev/null || :
"""


def _setup_script(image: str) -> str:
    return f"""\
depmod -a
modprobe configfs
modprobe nvmet
if ! modprobe nvmet-tcp; then
    echo 'ERROR: host kernel does not provide nvmet-tcp'
    echo 'ERROR: nvmfroot requires a host with NVMe/TCP target support'
    exit 1
fi
mountpoint -q /sys/kernel/config || mount -t configfs none /sys/kernel/config
test -d /sys/kernel/config/nvmet

{_cleanup_script()}

base=/sys/kernel/config/nvmet
subsys="$base/subsystems/{TARGET_NAME}"
port="$base/ports/1"
ns="$subsys/namespaces/1"

mkdir -p "$ns" "$port/subsystems"
echo 1 > "$subsys/attr_allow_any_host"
printf '%s' {shlex.quote(image)} > "$ns/device_path"
echo 1 > "$ns/enable"

echo ipv4 > "$port/addr_adrfam"
echo tcp > "$port/addr_trtype"
echo 0.0.0.0 > "$port/addr_traddr"
echo {TARGET_PORT} > "$port/addr_trsvcid"
ln -s "$subsys" "$port/subsystems/{TARGET_NAME}"

echo 'NVMF TARGET READY'
"""


def start(ctx: HostServiceContext) -> HostServiceProcess:
    image = ctx.top_workdir / "sysimage.img"
    if not image.exists():
        raise HostServiceError(f"missing NVMf backing image: {image}")

    logfile = ctx.top_logdir / "nvmf.log"
    logfile.parent.mkdir(parents=True, exist_ok=True)
    logfh = logfile.open("ab", buffering=0)

    ctx.message(f"starting nvmf service on 0.0.0.0:{TARGET_PORT}")

    try:
        _run_script(_setup_script(str(image)), logfh)
    except subprocess.CalledProcessError as exc:
        _run_cleanup(_cleanup_script(), logfh)
        logfh.close()
        raise HostServiceError(f"nvmf service did not start, see {logfile}") from exc

    if not wait_tcp_port("127.0.0.1", TARGET_PORT, 10):
        _run_cleanup(_cleanup_script(), logfh)
        logfh.close()
        raise HostServiceError(f"nvmf service did not listen, see {logfile}")

    proc = subprocess.Popen(
        ["sleep", "infinity"],
        stdout=logfh,
        stderr=subprocess.STDOUT,
        start_new_session=True,
    )
    return HostServiceProcess(
        name="nvmf",
        process=proc,
        logfh=logfh,
        cleanup=lambda: _run_cleanup(_cleanup_script(), logfh),
    )
