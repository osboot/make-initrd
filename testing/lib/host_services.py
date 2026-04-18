# SPDX-License-Identifier: GPL-3.0-or-later

from __future__ import annotations

import socket
import subprocess
import time
from contextlib import contextmanager
from dataclasses import dataclass
from importlib import import_module
from pathlib import Path
from typing import BinaryIO, Callable, Iterator


class HostServiceError(RuntimeError):
    pass


@dataclass(frozen=True)
class HostServiceContext:
    topdir: Path
    top_workdir: Path
    top_logdir: Path
    vendor_name: str
    message: Callable[[str], None]
    install_package_set: Callable[[str], str]
    builddir: str
    workdir_rel: str


@dataclass
class HostServiceProcess:
    name: str
    process: subprocess.Popen[bytes]
    logfh: BinaryIO
    cleanup: Callable[[], None] | None = None


def wait_tcp_port(host: str, port: int, timeout: float) -> bool:
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
            sock.settimeout(1)
            if sock.connect_ex((host, port)) == 0:
                return True
        time.sleep(0.2)
    return False


def wait_log_line(path: Path, text: str, timeout: float) -> bool:
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        try:
            if text in path.read_text(errors="ignore"):
                return True
        except FileNotFoundError:
            pass
        time.sleep(0.2)
    return False


def service_module(name: str):
    module_name = name.replace("-", "_")
    try:
        return import_module(f"lib.services.{module_name}")
    except ModuleNotFoundError as exc:
        if exc.name != f"lib.services.{module_name}":
            raise
        raise HostServiceError(f"unsupported host service: {name}") from exc


def start_service(ctx: HostServiceContext, name: str) -> HostServiceProcess:
    return service_module(name).start(ctx)


def prepare_services(ctx: HostServiceContext, names: list[str]) -> None:
    for name in names:
        prepare = getattr(service_module(name), "prepare", None)
        if prepare is not None:
            prepare(ctx)


def package_sets(names: list[str]) -> list[str]:
    result: list[str] = []
    for name in names:
        for package_set in getattr(service_module(name), "PACKAGE_SETS", ()):
            if package_set not in result:
                result.append(package_set)
    return result


def configure_initrd(ctx: HostServiceContext, names: list[str], initrd_mk: list[str]) -> list[str]:
    result = list(initrd_mk)
    for name in names:
        configure = getattr(service_module(name), "configure_initrd", None)
        if configure is not None:
            configure(ctx, result)
    return result


@contextmanager
def run_services(ctx: HostServiceContext, names: list[str]) -> Iterator[None]:
    processes: list[HostServiceProcess] = []
    try:
        for name in names:
            processes.append(start_service(ctx, name))
        yield
    finally:
        for service in reversed(processes):
            service.process.terminate()
            try:
                service.process.wait(timeout=10)
            except subprocess.TimeoutExpired:
                service.process.kill()
                service.process.wait()
            if service.cleanup is not None:
                service.cleanup()
            service.logfh.close()


def print_logs(ctx: HostServiceContext, names: list[str]) -> None:
    for name in names:
        log_path = ctx.top_logdir / f"{name}.log"
        if not log_path.exists():
            continue
        print(f"<<<<<< {log_path} <<<<<<", flush=True)
        print(log_path.read_text(errors="ignore"), end="", flush=True)
        print(">>>>>>", flush=True)
