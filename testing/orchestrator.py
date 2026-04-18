#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later

from __future__ import annotations

import os
import pty
import re
import shlex
import signal
import select
import subprocess
import sys
import threading
import time
import tomllib
from importlib import import_module
from contextlib import contextmanager
from dataclasses import dataclass
from datetime import datetime, timezone
from lib.build_api import BuildRenderContext
from lib.host_services import (
    configure_initrd,
    HostServiceContext,
    HostServiceError,
    package_sets,
    prepare_services,
    print_logs,
    run_services,
)
from lib.image_api import ImageRenderContext
from lib.package_api import PackageSet
from lib.template import render_template
from pathlib import Path
from typing import BinaryIO


DEFAULT_STEPS = [
    "create-images",
    "build-sources",
    "build-sysimage",
    "build-kickstart",
    "run-kickstart",
    "run-boot",
]

KNOWN_FATAL_ERRORS = [
    "(or press Control-D to continue):",
    "/.initrd/uevent/queues/udev.startup: No such file or directory",
    "rdshell: The waiting time expired!",
    "rdshell: The init program `/sbin/init' not found in the root directory.",
]

LUKS_PROMPT_RE = re.compile(rb"Enter passphrase for /dev/.*: ")

TEST_PARAM_KEYS = [
    "kickstart_disks",
    "kickstart_sdcards",
    "boot_disks",
    "boot_sdcards",
    "boot_cmdline",
    "boot_prog",
    "boot_bios",
    "boot_logs",
    "direct_boot",
    "direct_boot_features",
    "host_services",
    "qemu_memory",
    "qemu_timeout",
    "qemu_disk_format",
    "qemu_sdcard_format",
]

TEST_PARAM_ENV = {
    "QEMU_MEMORY": "qemu_memory",
    "QEMU_TIMEOUT": "qemu_timeout",
    "QEMU_DISK_FORMAT": "qemu_disk_format",
    "QEMU_SDCARD_FORMAT": "qemu_sdcard_format",
}
TEST_PARAM_ENV = {value: key for key, value in TEST_PARAM_ENV.items()}

IGNORE_FIRMWARE_EXPR = (
    " '(' "
    " -path '*/firmware/amdgpu/*' -o "
    " -path '*/firmware/intel/*' -o "
    " -path '*/firmware/iwlwifi-*' -o "
    " -path '*/firmware/mellanox/*' -o "
    " -path '*/firmware/mrvl/*' -o "
    " -path '*/firmware/nvidia/*' -o "
    " -path '*/firmware/qcom/*' -o "
    " -path '*/firmware/i915/*' "
    " ')'"
)

IGNORE_KMODULES_EXPR = (
    " '(' "
    " -path '*/kernel/drivers/bluetooth/*' -o "
    " -path '*/kernel/drivers/gpio/*' -o "
    " -path '*/kernel/drivers/gpu/*' -o "
    " -path '*/kernel/drivers/hwmon/*' -o "
    " -path '*/kernel/drivers/hwtracing/*' -o "
    " -path '*/kernel/drivers/i2c/*' -o "
    " -path '*/kernel/drivers/iio/*' -o "
    " -path '*/kernel/drivers/input/joystick/*' -o "
    " -path '*/kernel/drivers/input/misc/*' -o "
    " -path '*/kernel/drivers/input/touchscreen/*' -o "
    " -path '*/kernel/drivers/isdn/*' -o "
    " -path '*/kernel/drivers/leds/*' -o "
    " -path '*/kernel/drivers/media-core/*' -o "
    " -path '*/kernel/drivers/net/can/*' -o "
    " -path '*/kernel/drivers/net/dsa/*' -o "
    " -path '*/kernel/drivers/net/hamradio/*' -o "
    " -path '*/kernel/drivers/net/ieee802154/*' -o "
    " -path '*/kernel/drivers/net/usb/*' -o "
    " -path '*/kernel/drivers/net/wan/*' -o "
    " -path '*/kernel/drivers/net/wimax/*' -o "
    " -path '*/kernel/drivers/net/wireless/*' -o "
    " -path '*/kernel/drivers/net/wwan/*' -o "
    " -path '*/kernel/drivers/nfc/*' -o "
    " -path '*/kernel/drivers/pcmcia/*' -o "
    " -path '*/kernel/drivers/power/*' -o "
    " -path '*/kernel/drivers/usb/*' -o "
    " -path '*/kernel/drivers/video/*' -o "
    " -path '*/kernel/drivers/watchdog/*' -o "
    " -path '*/kernel/drivers/xen/*' -o "
    " -path '*/kernel/sound/*' "
    " ')'"
)


@dataclass(frozen=True)
class VendorSpec:
    name: str
    image: str
    arch: str = "x86_64"


VENDORS = {
    "fedora": VendorSpec("fedora", "fedora:latest"),
    "ubuntu": VendorSpec("ubuntu", "ubuntu:latest"),
    "altlinux": VendorSpec("altlinux", "alt:sisyphus"),
    "gentoo": VendorSpec("gentoo", "gentoo/stage3:systemd"),
    "gentoo_musl": VendorSpec("gentoo_musl", "gentoo/stage3:musl"),
}


class OrchestratorError(RuntimeError):
    pass


class FatalStepError(OrchestratorError):
    pass


def vendor_module(vendor: str, area: str):
    module_name = f"lib.vendor.{vendor}.{area}"
    try:
        return import_module(module_name)
    except ModuleNotFoundError as exc:
        if exc.name not in {"lib.vendor", f"lib.vendor.{vendor}", module_name}:
            raise
        raise FatalStepError(f"missing vendor module for {vendor}: {area}") from exc


def message(ctx: "Context", text: str) -> None:
    prefix = Path(sys.argv[0]).name
    if ctx.testname:
        prefix += f" ({ctx.testname})"
    print(f"{prefix}: {text}", file=sys.stderr, flush=True)


def gh_group_start(title: str) -> None:
    print(f"::group::{title}", flush=True)


def gh_group_end() -> None:
    print("::endgroup::", flush=True)


@contextmanager
def gh_group(title: str):
    gh_group_start(title)
    try:
        yield
    finally:
        gh_group_end()


def parse_duration(value: str) -> float:
    if not value:
        return 300.0
    match = re.fullmatch(r"(\d+)([smh]?)", value)
    if not match:
        raise FatalStepError(f"invalid timeout value: {value}")
    amount = int(match.group(1))
    unit = match.group(2) or "s"
    scale = {"s": 1, "m": 60, "h": 3600}[unit]
    return float(amount * scale)


def load_test_params(topdir: Path, testname: str) -> dict[str, str]:
    test_file = topdir / "testing" / f"{testname}.cfg"
    return load_toml_test_params(test_file)


def load_toml_test_params(test_file: Path) -> dict[str, str]:
    lines: list[str] = []
    for line in test_file.read_text().splitlines():
        if not line.startswith("#"):
            break
        line = line[1:]
        if line.startswith(" "):
            line = line[1:]
        lines.append(line)

    if "[test]" not in lines:
        raise FatalStepError(f"missing [test] metadata in {test_file}")

    data = tomllib.loads("\n".join(lines))
    test = data.get("test", {})
    if not isinstance(test, dict):
        raise FatalStepError(f"invalid [test] metadata in {test_file}")

    result = {name: "" for name in TEST_PARAM_KEYS}
    for key, value in test.items():
        if key not in result:
            raise FatalStepError(f"unknown test metadata key in {test_file}: {key}")
        if key in {"boot_cmdline", "boot_logs", "host_services"} and isinstance(value, list):
            value = " ".join(str(item) for item in value)
        elif isinstance(value, bool):
            value = "1" if value else "0"
        elif isinstance(value, (int, float, str)):
            value = str(value)
        else:
            raise FatalStepError(f"invalid value for {key} in {test_file}")
        result[key] = value
    return result


def write_executable(path: Path, content: str) -> None:
    lines = content.lstrip().splitlines()
    path.write_text("\n".join(line.lstrip() for line in lines) + "\n")
    path.chmod(0o755)


def run_command(ctx: "Context", argv: list[str], *, input_text: str | None = None) -> None:
    message(ctx, f"run: {shlex.join(argv)}")
    subprocess.run(argv, cwd=ctx.topdir, check=True, text=True, input=input_text)


def image_depmod() -> str:
    return (
        "find /lib/modules -mindepth 1 -maxdepth 1 -type d -printf '%f\\n' "
        "| xargs -rI '{}' -- depmod -a '{}';"
    )


def dockerfiles(ctx: "Context") -> tuple[str, str]:
    render_ctx = ImageRenderContext(
        vendor_name=ctx.vendor.name,
        base_image=ctx.vendor.image,
        arch=ctx.vendor.arch,
        packages=ctx.packages,
        depmod_cmd=image_depmod(),
        ignore_firmware_expr=IGNORE_FIRMWARE_EXPR,
        ignore_kmodules_expr=IGNORE_KMODULES_EXPR,
        cpu_count=max(1, (os.cpu_count() or 1) * 2),
    )
    return vendor_module(ctx.vendor.name, "image").render_dockerfiles(render_ctx)


def build_sources_script(ctx: "Context") -> str:
    build_ctx = BuildRenderContext(
        builddir=ctx.builddir,
        workdir_rel=ctx.workdir_rel,
        top_workdir=str(ctx.top_workdir),
    )
    return vendor_module(ctx.vendor.name, "build").render_build_sources_script(build_ctx)


def build_kickstart_script(ctx: "Context") -> str:
    build_ctx = BuildRenderContext(
        builddir=ctx.builddir,
        workdir_rel=ctx.workdir_rel,
        top_workdir=str(ctx.top_workdir),
    )
    return vendor_module(ctx.vendor.name, "build").render_build_kickstart_script(build_ctx)


def package_install_command(ctx: "Context", package_set: str) -> str:
    packages = ctx.packages.packages(package_set)
    if not packages:
        return ":"
    return vendor_module(ctx.vendor.name, "packages").install_command(packages)


def host_service_install_command(ctx: "Context") -> str:
    commands = [
        package_install_command(ctx, package_set)
        for package_set in package_sets(ctx.host_services)
    ]
    return "; ".join(commands) if commands else ":"


def build_direct_boot_script(ctx: "Context") -> str:
    spec = vendor_module(ctx.vendor.name, "build").boot_script_spec()

    features = ctx.param("direct_boot_features")
    if not features:
        features = "add-modules add-udev-rules qemu rdshell modules-virtio modules-blockdev"

    initrd_mk = [
        f"IMAGEFILE = {ctx.builddir}/{ctx.workdir_rel}/boot-initrd.img",
        "AUTODETECT =",
        "DISABLE_FEATURES += ucode",
        f"FEATURES = {features}",
        "MODULES_ADD += e1000",
        "MODULES_PRELOAD += e1000 virtio-pci",
    ]

    initrd_mk = configure_initrd(host_service_context(ctx), ctx.host_services, initrd_mk)
    install_services = host_service_install_command(ctx) if spec.install_services else ":"

    return render_template(
        "direct-boot.sh.in",
        {
            "BUILDDIR": shlex.quote(ctx.builddir),
            "INITRD_MK": "\n".join(initrd_mk),
            "INSTALL_SERVICES": install_services,
            "BOOT_KERNEL_SRC": spec.boot_kernel_src,
            "BOOT_KERNEL_DST": f"{ctx.builddir}/{ctx.workdir_rel}/boot-vmlinuz",
        },
    )


def pack_sysimage_script(ctx: "Context") -> str:
    return render_template(
        "pack-sysimage.sh.in",
        {
            "WORKDIR_REL": ctx.workdir_rel,
        },
    )


@dataclass
class Context:
    topdir: Path
    vendor: VendorSpec
    testname: str
    packages: PackageSet
    test_params: dict[str, str]
    builddir: str = "/srv"

    @property
    def workdir_rel(self) -> str:
        suffix = f"/{self.testname}" if self.testname else "/"
        return f"testing/cache/{self.vendor.name}{suffix}"

    @property
    def logdir_rel(self) -> str:
        suffix = f"/{self.testname}" if self.testname else "/"
        return f"testing/logs/{self.vendor.name}{suffix}"

    @property
    def statusdir(self) -> Path:
        return self.topdir / "testing" / "status"

    @property
    def top_workdir(self) -> Path:
        return self.topdir / self.workdir_rel

    @property
    def top_logdir(self) -> Path:
        return self.topdir / self.logdir_rel

    def param(self, name: str, default: str = "") -> str:
        value = self.test_params.get(name)
        if value not in {None, ""}:
            return value
        value = os.environ.get(name)
        if value in {None, ""}:
            value = os.environ.get(TEST_PARAM_ENV.get(name, ""))
        return default if value in {None, ""} else value

    def flag(self, name: str) -> bool:
        return self.param(name) in {"1", "true", "yes", "on"}

    @property
    def direct_boot(self) -> bool:
        return self.flag("direct_boot")

    @property
    def host_services(self) -> list[str]:
        return self.param("host_services").split()


def prepare_testsuite(ctx: Context) -> None:
    ctx.top_workdir.mkdir(parents=True, exist_ok=True)
    ctx.top_logdir.mkdir(parents=True, exist_ok=True)
    ctx.statusdir.mkdir(parents=True, exist_ok=True)

    if not ctx.testname:
        return

    (ctx.statusdir / f"{ctx.vendor.name}-{ctx.testname}").write_text("FAILED\n")

    if ctx.direct_boot:
        return

    variant = os.environ.get("VARIANT", "")
    variant_suffix = f"-{variant}" if variant else ""
    testing = ctx.topdir / "testing"

    ks_testcase = testing / f"{ctx.testname}.cfg"
    ks_sysimage = testing / f"ks-{ctx.vendor.name}{variant_suffix}-sysimage.cfg"
    ks_initrd = testing / f"ks-{ctx.vendor.name}{variant_suffix}-initrd.cfg"
    ks_done = testing / f"ks-{ctx.vendor.name}{variant_suffix}-done.cfg"

    if not ks_sysimage.exists():
        ks_sysimage = testing / "ks-sysimage.cfg"
    if not ks_initrd.exists():
        ks_initrd = testing / "ks-initrd.cfg"
    if not ks_done.exists():
        ks_done = testing / "ks-done.cfg"

    cmdline = ctx.param("boot_cmdline")
    parts = []
    for src in (ks_sysimage, ks_testcase, ks_initrd, ks_done):
        parts.append(src.read_text().replace("@CMDLINE@", cmdline))
    (ctx.top_workdir / "ks.cfg").write_text("".join(parts))


def qemu_cpu_args(ctx: Context) -> list[str]:
    args: list[str] = []
    if not Path("/dev/kvm").exists():
        ncpu = max(1, (os.cpu_count() or 1) * 2)
        args.extend(["-smp", str(ncpu), "-accel", "tcg,thread=multi,tb-size=1024"])
    else:
        args.extend(["-enable-kvm", "-cpu", "host"])

    if ctx.param("boot_bios", "pc") == "efi":
        efivars = ctx.top_workdir / "efivars.bin"
        if not efivars.exists():
            subprocess.run(
                ["cp", "-vTLf", "/usr/share/OVMF/OVMF_VARS_4M.fd", str(efivars)],
                check=True,
            )
        args.extend(
            [
                "-machine", "q35",
                "-drive", "if=pflash,format=raw,readonly=on,file=/usr/share/OVMF/OVMF_CODE_4M.fd",
                "-drive", f"if=pflash,format=raw,file={efivars}",
            ]
        )
    else:
        args.extend(["-machine", "pc"])
    return args


def ensure_disk(path: Path, fmt: str) -> None:
    if path.exists():
        return
    subprocess.run(["qemu-img", "create", "-q", "-f", fmt, str(path), "4G"], check=True)


def qemu_disk_args(index: int, path: Path, fmt: str) -> list[str]:
    return [
        "-object", f"iothread,id=iothread{index}",
        "-blockdev", f"node-name=file{index},driver=file,filename={path},aio=native,auto-read-only=on,discard=unmap,cache.direct=on,cache.no-flush=off",
        "-blockdev", f"node-name=drive{index},driver={fmt},file=file{index}",
        "-device", f"virtio-blk-pci,drive=drive{index},iothread=iothread{index},num-queues=1",
    ]


def qemu_sdcard_args(index: int, path: Path, fmt: str) -> list[str]:
    return [
        "-device", f"sd-card,drive=sdcard-{index}",
        "-drive", f"id=sdcard-{index},if=none,format={fmt},file={path}",
    ]


def build_qemu_args(ctx: Context, *, kickstart: bool) -> list[str]:
    args = [f"qemu-system-{ctx.vendor.arch}"]
    args.extend(["-m", ctx.param("qemu_memory", "1G"), "-no-reboot"])
    if kickstart:
        boot_kernel = ctx.top_workdir / "boot-ks-vmlinuz"
        boot_initrd = ctx.top_workdir / "boot-ks-initrd.img"
        boot_append = "console=ttyS0,115200n8 quiet rdlog=console ksfile=ks.cfg"
    elif ctx.direct_boot:
        boot_kernel = ctx.top_workdir / "boot-vmlinuz"
        boot_initrd = ctx.top_workdir / "boot-initrd.img"
        boot_append = f"console=ttyS0,115200n8 {ctx.param('boot_cmdline')}"
    else:
        boot_kernel = None
        boot_initrd = None
        boot_append = None

    if boot_kernel:
        args.extend(["-kernel", str(boot_kernel)])
    if boot_initrd:
        args.extend(["-initrd", str(boot_initrd)])
    if boot_append:
        args.extend(["-append", boot_append])

    args.extend(["-rtc", "base=localtime,clock=vm"])
    args.extend(["-nic", "user,model=e1000"])
    args.extend(["-nographic", "-serial", "chardev:stdio", "-chardev", "stdio,mux=on,id=stdio,signal=off"])
    args.extend(["-mon", "chardev=stdio,mode=readline"])
    args.extend(["-pidfile", str(ctx.top_workdir / "qemu.pid")])
    args.extend(qemu_cpu_args(ctx))
    args.extend(["-device", "virtio-scsi-pci,id=scsi"])
    args.extend(["-device", "sdhci-pci"])

    disk_fmt = ctx.param("qemu_disk_format", "qcow2")
    sd_fmt = ctx.param("qemu_sdcard_format", "qcow2")

    if kickstart:
        message(ctx, f"attach kickstart sysimage: {ctx.top_workdir / 'sysimage.img'}")
        args.extend(qemu_disk_args(0, ctx.top_workdir / "sysimage.img", "raw"))
        for i in range(1, int(ctx.param("kickstart_disks", "0")) + 1):
            path = ctx.top_workdir / f"disk-{i}.img"
            message(ctx, f"prepare kickstart disk {i}: {path} ({disk_fmt})")
            ensure_disk(path, disk_fmt)
            args.extend(qemu_disk_args(i, path, disk_fmt))
        for i in range(1, int(ctx.param("kickstart_sdcards", "0")) + 1):
            path = ctx.top_workdir / f"sdcard-{i}.img"
            message(ctx, f"prepare kickstart sdcard {i}: {path} ({sd_fmt})")
            ensure_disk(path, disk_fmt)
            args.extend(qemu_sdcard_args(i, path, sd_fmt))
    else:
        for i in range(1, int(ctx.param("boot_disks", "0")) + 1):
            message(ctx, f"attach boot disk {i}: {ctx.top_workdir / f'disk-{i}.img'} ({disk_fmt})")
            args.extend(qemu_disk_args(i, ctx.top_workdir / f"disk-{i}.img", disk_fmt))
        for i in range(1, int(ctx.param("boot_sdcards", "0")) + 1):
            message(ctx, f"attach boot sdcard {i}: {ctx.top_workdir / f'sdcard-{i}.img'} ({sd_fmt})")
            args.extend(qemu_sdcard_args(i, ctx.top_workdir / f"sdcard-{i}.img", sd_fmt))

    return args


def qemu_calc_boot_duration(log_path: Path) -> int:
    pattern = re.compile(r"^\[([0-9-]{10} [0-9:]{8})\] ")
    timestamps: list[int] = []
    enabled = False
    for line in log_path.read_text(errors="ignore").splitlines():
        if not enabled and re.match(r"^Booting from (Hard Disk|ROM)\.\.\.", line):
            enabled = True
        if not enabled:
            continue
        match = pattern.match(line)
        if not match:
            continue
        dt = datetime.strptime(match.group(1), "%Y-%m-%d %H:%M:%S").replace(tzinfo=timezone.utc)
        ts = int(dt.timestamp())
        if not timestamps or timestamps[-1] != ts:
            timestamps.append(ts)
    if len(timestamps) < 2:
        return 0
    return timestamps[-1] - timestamps[0]


def valid_log(log_path: Path) -> bool:
    return any(line.startswith("IT WORKS!") for line in log_path.read_text(errors="ignore").splitlines())


def write_qemu_output(logfh: BinaryIO, data: bytes) -> None:
    logfh.write(data)
    sys.stdout.buffer.write(data)
    sys.stdout.buffer.flush()


def qemu_watchdog(ctx: Context, logfile: Path, stop_event: threading.Event) -> None:
    for _ in range(20):
        if stop_event.is_set() or (ctx.top_workdir / "qemu.pid").exists():
            break
        time.sleep(1)
    else:
        return

    while not stop_event.is_set():
        try:
            content = logfile.read_text(errors="ignore")
        except FileNotFoundError:
            time.sleep(1)
            continue

        if any(err in content for err in KNOWN_FATAL_ERRORS):
            pidfile = ctx.top_workdir / "qemu.pid"
            if pidfile.exists():
                try:
                    os.kill(int(pidfile.read_text().strip()), signal.SIGKILL)
                except (OSError, ValueError):
                    pass
            error_message = (
                "\n\nERROR!!!\n"
                "The sign of a fatal error was found in the log.\n"
                "There is no reason to wait for a timeout.\n\n"
            )
            with logfile.open("a") as fh:
                fh.write(error_message)
            print(error_message, end="", flush=True)
            break
        time.sleep(1)


def host_service_context(ctx: Context) -> HostServiceContext:
    return HostServiceContext(
        topdir=ctx.topdir,
        top_workdir=ctx.top_workdir,
        top_logdir=ctx.top_logdir,
        vendor_name=ctx.vendor.name,
        message=lambda text: message(ctx, text),
        install_package_set=lambda package_set: package_install_command(ctx, package_set),
        builddir=ctx.builddir,
        workdir_rel=ctx.workdir_rel,
    )


@contextmanager
def running_host_services(ctx: Context, *, kickstart: bool):
    if kickstart:
        yield
        return

    try:
        with run_services(host_service_context(ctx), ctx.host_services):
            yield
    except Exception:
        print_host_service_logs(ctx)
        raise


def print_host_service_logs(ctx: Context) -> None:
    print_logs(host_service_context(ctx), ctx.host_services)


def qemu_exec(ctx: Context, *, step_name: str, kickstart: bool) -> None:
    logfile = ctx.top_logdir / "qemu.log"
    logfile.parent.mkdir(parents=True, exist_ok=True)
    logfile.write_text("")
    qemu_cmd = build_qemu_args(ctx, kickstart=kickstart)
    boot_prog = "" if kickstart else ctx.param("boot_prog")
    if boot_prog not in {"", "expect-password"}:
        raise FatalStepError(f"unsupported boot_prog: {boot_prog}")
    command = qemu_cmd
    message(ctx, f"qemu log: {logfile}")
    message(ctx, f"qemu timeout: {ctx.param('qemu_timeout', '5m')}")
    message(ctx, f"qemu command: {shlex.join(command)}")
    print(f"<<<<<< {logfile} live output <<<<<<", flush=True)

    stop_event = threading.Event()
    watchdog = threading.Thread(target=qemu_watchdog, args=(ctx, logfile, stop_event), daemon=True)
    watchdog.start()

    timeout_value = parse_duration(ctx.param("qemu_timeout", "5m"))
    rc = 0
    qemu_started_at = time.monotonic()
    proc: subprocess.Popen[bytes] | None = None
    master_fd = -1
    slave_fd = -1
    try:
        with running_host_services(ctx, kickstart=kickstart):
            master_fd, slave_fd = pty.openpty()
            proc = subprocess.Popen(
                command,
                cwd=ctx.topdir,
                stdin=slave_fd,
                stdout=slave_fd,
                stderr=slave_fd,
                start_new_session=True,
            )
            os.close(slave_fd)
            slave_fd = -1
            start_time = time.monotonic()
            prompt_buffer = b""

            with logfile.open("ab", buffering=0) as logfh:
                while True:
                    now = time.monotonic()
                    if now - start_time >= timeout_value:
                        rc = 124
                        os.killpg(proc.pid, signal.SIGTERM)
                        break

                    timeout_left = min(1.0, timeout_value - (now - start_time))
                    ready, _, _ = select.select([master_fd], [], [], max(timeout_left, 0.0))
                    if ready:
                        try:
                            data = os.read(master_fd, 65536)
                        except OSError:
                            data = b""
                        if data:
                            write_qemu_output(logfh, data)
                            prompt_buffer = (prompt_buffer + data)[-8192:]
                            if boot_prog == "expect-password" and LUKS_PROMPT_RE.search(prompt_buffer):
                                time.sleep(1)
                                os.write(master_fd, b"qwerty\r")
                                prompt_buffer = b""
                        elif proc.poll() is not None:
                            break
                    elif proc.poll() is not None:
                        break

                if rc == 124:
                    try:
                        proc.wait(timeout=5)
                    except subprocess.TimeoutExpired:
                        os.killpg(proc.pid, signal.SIGKILL)
                        proc.wait()
                else:
                    try:
                        proc.wait(timeout=1)
                    except subprocess.TimeoutExpired:
                        os.killpg(proc.pid, signal.SIGTERM)
                        try:
                            proc.wait(timeout=5)
                        except subprocess.TimeoutExpired:
                            os.killpg(proc.pid, signal.SIGKILL)
                            proc.wait()
                    rc = proc.returncode or 0
    finally:
        if master_fd >= 0:
            try:
                os.close(master_fd)
            except OSError:
                pass
        if slave_fd >= 0:
            try:
                os.close(slave_fd)
            except OSError:
                pass
        stop_event.set()
        watchdog.join(timeout=1)

    print(">>>>>>", flush=True)

    if not valid_log(logfile):
        message(ctx, "check FAIL")
        rc = rc or 1
    else:
        message(ctx, "check DONE")

    qemu_runtime = int(time.monotonic() - qemu_started_at)
    boot_duration = qemu_calc_boot_duration(logfile) if rc == 0 else 0
    artifact = ctx.statusdir / f"artifact-{ctx.vendor.name}-{ctx.testname}.txt"
    artifact.write_text(
        f'vendor="{ctx.vendor.name}"\n'
        f'testname="{ctx.testname}"\n'
        f'step="{step_name}"\n'
        f"duration={boot_duration}\n"
        f"boot_duration={boot_duration}\n"
        f"qemu_runtime={qemu_runtime}\n"
        f"status={rc}\n"
    )

    for entry in ctx.param("boot_logs").split():
        path = Path(entry)
        if path.is_file():
            print(f"<<<<<< {entry} <<<<<<")
            print(path.read_text(errors="ignore"), end="")
            print(">>>>>>")

    print_host_service_logs(ctx)

    if rc != 0:
        message(ctx, f"qemu failed with status {rc}")
        raise subprocess.CalledProcessError(rc, command)


def step_create_images(ctx: Context) -> None:
    sysimage_df, devel_df = dockerfiles(ctx)
    with gh_group("create sysimage image"):
        run_command(
            ctx,
            [
                "podman", "image", "build", "--squash", "--force-rm", "--pull=missing",
                "-t", f"localhost/image-{ctx.vendor.name}:sysimage",
                "-f", "-",
            ],
            input_text=sysimage_df,
        )

    with gh_group("create devel image"):
        run_command(
            ctx,
            [
                "podman", "image", "build", "--squash", "--force-rm", "--pull=never",
                "-t", f"localhost/image-{ctx.vendor.name}:devel",
                "-f", "-",
            ],
            input_text=devel_df,
        )


def step_build_sources(ctx: Context) -> None:
    with gh_group("building from git tree"):
        run_sh = ctx.top_workdir / "run.sh"
        write_executable(run_sh, build_sources_script(ctx))
        run_command(
            ctx,
            [
                "podman", "run", "--rm", "-i",
                f"--volume={ctx.topdir}:{ctx.builddir}",
                f"localhost/image-{ctx.vendor.name}:devel",
                f"{ctx.builddir}/{ctx.workdir_rel}/run.sh",
            ],
        )


def step_build_sysimage(ctx: Context) -> None:
    with gh_group("packing sysimage"):
        run_sh = ctx.top_workdir / "run.sh"
        write_executable(run_sh, pack_sysimage_script(ctx))
        run_command(
            ctx,
            [
                "podman", "run", "--rm", "-i",
                "--mount", f"type=image,src=localhost/image-{ctx.vendor.name}:sysimage,dst=/image",
                "--mount", f"type=tmpfs,destination=/image{ctx.builddir}",
                "--mount", f"type=bind,src={ctx.topdir}/.build/dest,dst=/image{ctx.builddir}/.build/dest",
                "--mount", f"type=bind,src={ctx.topdir}/features,dst=/image{ctx.builddir}/features",
                "--mount", f"type=bind,src={ctx.topdir}/guess,dst=/image{ctx.builddir}/guess",
                "--mount", f"type=bind,src={ctx.topdir}/tools,dst=/image{ctx.builddir}/tools",
                f"--volume={ctx.topdir}:/host",
                f"localhost/image-{ctx.vendor.name}:devel",
                f"/host/{ctx.workdir_rel}/run.sh",
            ],
        )
    if ctx.direct_boot:
        with gh_group("creating direct boot image"):
            prepare_services(host_service_context(ctx), ctx.host_services)
            run_sh = ctx.top_workdir / "run.sh"
            write_executable(run_sh, build_direct_boot_script(ctx))
            run_command(
                ctx,
                [
                    "podman", "run", "--rm", "-i",
                    f"--volume={ctx.topdir}:{ctx.builddir}",
                    f"localhost/image-{ctx.vendor.name}:devel",
                    f"{ctx.builddir}/{ctx.workdir_rel}/run.sh",
                ],
            )
            for artifact in ("boot-vmlinuz", "boot-initrd.img"):
                path = ctx.top_workdir / artifact
                if not path.exists():
                    raise FatalStepError(f"direct boot build did not create {path}")
                message(ctx, f"created {path}")


def step_build_kickstart(ctx: Context) -> None:
    if ctx.direct_boot:
        message(ctx, "skip build-kickstart for direct boot test")
        return

    with gh_group("creating kickstart image"):
        run_sh = ctx.top_workdir / "run.sh"
        write_executable(run_sh, build_kickstart_script(ctx))
        run_command(
            ctx,
            [
                "podman", "run", "--rm", "-i",
                f"--volume={ctx.topdir}:{ctx.builddir}",
                f"localhost/image-{ctx.vendor.name}:devel",
                f"{ctx.builddir}/{ctx.workdir_rel}/run.sh",
            ],
        )
        for artifact in ("boot-ks-vmlinuz", "boot-ks-initrd.img"):
            path = ctx.top_workdir / artifact
            if not path.exists():
                raise FatalStepError(f"build-kickstart did not create {path}")
            message(ctx, f"created {path}")


def step_run_kickstart(ctx: Context) -> None:
    if ctx.direct_boot:
        message(ctx, "skip run-kickstart for direct boot test")
        return

    with gh_group("creating qemu kickstart disks"):
        qemu_exec(ctx, step_name="run-kickstart", kickstart=True)


def step_run_boot(ctx: Context) -> None:
    with gh_group("checking the ability to boot"):
        qemu_exec(ctx, step_name="run-boot", kickstart=False)


STEP_HANDLERS = {
    "create-images": step_create_images,
    "build-sources": step_build_sources,
    "build-sysimage": step_build_sysimage,
    "build-kickstart": step_build_kickstart,
    "run-kickstart": step_run_kickstart,
    "run-boot": step_run_boot,
}


def parse_args(argv: list[str]) -> tuple[VendorSpec, str, list[str]]:
    if len(argv) < 2:
        raise FatalStepError("usage: testing/orchestrator.py <vendor> [<testname>] <step>...")

    vendor_name = argv[1]
    if vendor_name not in VENDORS:
        raise FatalStepError(f"unknown vendor: {vendor_name}")
    vendor = VENDORS[vendor_name]

    rest = argv[2:]
    if len(rest) > 1:
        testname = rest[0]
        steps = rest[1:]
    else:
        testname = ""
        steps = rest

    if not steps:
        raise FatalStepError("no steps given")
    if steps == ["all"]:
        if not testname:
            raise FatalStepError("the 'all' step requires a testcase")
        steps = list(DEFAULT_STEPS)

    return vendor, testname, steps


def main(argv: list[str]) -> int:
    try:
        vendor, testname, steps = parse_args(argv)
        topdir = Path(__file__).resolve().parent.parent
        packages = vendor_module(vendor.name, "packages").get_packages()
        test_params = load_test_params(topdir, testname) if testname else {}
        ctx = Context(topdir=topdir, vendor=vendor, testname=testname, packages=packages, test_params=test_params)
        prepare_testsuite(ctx)

        for step in steps:
            handler = STEP_HANDLERS.get(step)
            if handler is None:
                raise FatalStepError(f"unknown step: {step}")
            handler(ctx)
    except (FatalStepError, HostServiceError) as exc:
        print(f"{Path(argv[0]).name}: {exc}", file=sys.stderr)
        return 1
    except subprocess.CalledProcessError as exc:
        return exc.returncode or 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
