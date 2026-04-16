# SPDX-License-Identifier: GPL-3.0-or-later

from __future__ import annotations

import subprocess
from pathlib import Path

from lib.host_services import HostServiceContext, HostServiceError, HostServiceProcess, wait_tcp_port


PACKAGE_SETS = ("sshfs_service",)


def workdir(ctx: HostServiceContext) -> Path:
    return ctx.top_workdir / "sshfsroot"


def ensure_key(ctx: HostServiceContext) -> Path:
    root = workdir(ctx)
    key = root / "id_key"
    if key.exists():
        return key
    root.mkdir(parents=True, exist_ok=True)
    subprocess.run(["ssh-keygen", "-q", "-t", "ed25519", "-N", "", "-f", str(key)], check=True)
    return key


def prepare(ctx: HostServiceContext) -> None:
    ensure_key(ctx)


def configure_initrd(ctx: HostServiceContext, initrd_mk: list[str]) -> None:
    initrd_mk.append("FEATURES += sshfsroot")
    initrd_mk.append(f"SSHFS_KEY = {ctx.builddir}/{ctx.workdir_rel}/sshfsroot/id_key")


def start(ctx: HostServiceContext) -> HostServiceProcess:
    key = ensure_key(ctx)
    pubkey = Path(f"{key}.pub")
    root = workdir(ctx)
    logfile = ctx.top_logdir / "sshfsroot.log"
    logfile.parent.mkdir(parents=True, exist_ok=True)
    logfh = logfile.open("ab", buffering=0)

    ctx.message("starting sshfsroot service on 0.0.0.0:2222")
    proc = subprocess.Popen(
        [
            "podman", "run", "--rm", "--network=host",
            "--volume", f"{pubkey}:/tmp/sshfsroot-authorized-key:ro",
            f"localhost/image-{ctx.vendor_name}:sysimage",
            "sh", "-ec",
            ""
            f"{ctx.install_package_set('sshfs_service')}; "
            ""
            "if command -v useradd >/dev/null; then "
            "    useradd -M -U -p '' -d / -s /bin/false sftp; "
            "else "
            "    printf 'sftp:x:1000:1000:sftp:/:/bin/false\\n' >> /etc/passwd; "
            "    printf 'sftp:x:1000:\\n' >> /etc/group; "
            "    printf 'sftp::0:0:99999:7:::\\n' >> /etc/shadow 2>/dev/null || :; "
            "fi; "
            "passwd -d sftp >/dev/null 2>&1 || :; "
            "chmod 755 /; "
            "test ! -d /var/empty || chmod 755 /var/empty; "
            ""
            "install -d -m 755 /etc/openssh/authorized_keys /run/sshd; "
            "cat /tmp/sshfsroot-authorized-key > /etc/openssh/authorized_keys/sftp; "
            "chmod 644 /etc/openssh/authorized_keys/sftp; "
            ""
            "cat > /init.once <<'EOF'\n"
            "#!/bin/sh\n"
            "printf '\\n\\n\\n%s\\n\\n\\n' 'IT WORKS!'\n"
            "sync; reboot -f || poweroff -f || halt -f\n"
            "exit 0\n"
            "EOF\n"
            ""
            "chmod 755 /init.once; "
            ""
            "cat > /etc/sshd_config <<'EOF'\n"
            "Port 2222\n"
            "ListenAddress 0.0.0.0\n"
            "PasswordAuthentication no\n"
            "PubkeyAuthentication yes\n"
            "AuthorizedKeysFile /etc/openssh/authorized_keys/%u\n"
            "PidFile /tmp/sshd.pid\n"
            "LogLevel DEBUG3\n"
            "Subsystem sftp internal-sftp\n"
            "EOF\n"
            ""
            "test -e /init.once; "
            "ssh-keygen -A; "
            "exec /usr/sbin/sshd -D -e -f /etc/sshd_config",
        ],
        cwd=ctx.topdir,
        stdout=logfh,
        stderr=subprocess.STDOUT,
        start_new_session=True,
    )

    if not wait_tcp_port("127.0.0.1", 2222, 180):
        proc.terminate()
        try:
            proc.wait(timeout=5)
        except subprocess.TimeoutExpired:
            proc.kill()
            proc.wait()
        logfh.close()
        raise HostServiceError(f"sshfsroot service did not start, see {logfile}")

    return HostServiceProcess(name="sshfsroot", process=proc, logfh=logfh)
