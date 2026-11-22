# Feature: crash

This feature provides support for building lightweight **capture initramfs**
images used in kdump/crashdump environments (`crashkernel=`).

When a Linux kernel panics and `crash_kexec` boots into the secondary capture
kernel, this feature handles `/proc/vmcore` to:
1. Perform automated crash triage or launch an interactive shell with the `crash` utility.
2. Collect and save compressed vmcore dumps (`makedumpfile`) and kernel log buffers (`vmcore-dmesg`).
3. Stream or save dumps to **virtio-serial** (ideal for virtual machines), **EFI System Partition (ESP)**, or **disk partitions**.

## Design and Footprint Optimization

Capture kernels operate with severe memory constraints (typically 128M–512M
reserved by `crashkernel=`). To guarantee boot reliability and avoid OOM panics:

1. **Lightweight Device Management (`device-daemon-mdev`)**:
   Instead of launching heavy `udevd` with hundreds of rules and large `hwdb.bin`,
   this feature disables `add-udev-rules` and utilizes `mdev` from BusyBox
   (via `features/device-daemon-mdev`) to scan and populate `/dev`.

2. **Full Triage vs Dump-Only Mode**:
   * **Live mode (`CRASH_ENABLE_LIVE=1`, default)**: Includes the `crash` utility
     and mounts the boot filesystem read-only (`ro,noload`) to access external `vmlinux`
     without packaging the debug kernel into the ramdisk.
   * **Dump-only mode (`CRASH_ENABLE_LIVE=0`)**: Excludes the large `crash` debugger
     (saving ~15 MB of binary and library footprint), relying on `makedumpfile` and
     `vmcore-dmesg` to quickly collect the dump and stream it out.

3. **RAM Protection & Direct Writing**:
   Writing multi-megabyte dumps into tmpfs `/run/` can easily cause OOM in small
   crash reservations. Dumps are streamed directly to virtio character devices or
   written directly to mounted target filesystems (ESP or disk) without staging
   in RAM.

4. **Multiple Independent Targets**:
   Multiple destinations can be specified simultaneously (e.g. streaming to virtio
   and writing to EFI at the same time).

## Kernel Command-line Parameters

The following parameters can be passed to the capture kernel (e.g. via `kexec -p ... --append="..."`):

### Report Selection
* `rd.crash.report=`: Report type to collect:
  * `dump` (default): Lightweight collection — extracts `dmesg` log (`vmcore-dmesg`)
    and generates automated triage report if `crash` and `vmlinux` are available.
  * `vmcore`: Full binary dump collection via `makedumpfile -c -d 31` (or `-F` stream).

### Target Destinations
* `rd.crash.virtio` (or `rd.crash.virtio=<port>`): Stream dump/report to a virtio-serial port.
  If omitted or set to `auto`, automatically selects the first available port matching
  `*dump*`, `*crash*`, `*vmcore*`, or `/dev/vport0p1`.
* `rd.crash.efi` (or `rd.crash.efi=<spec>`): Save dump/report directly to the EFI System Partition
  (auto-detected by standard ESP GUID or `LABEL=EFI`).
* `rd.crash.disk` (or `rd.crash.disk=<spec>`): Mount filesystem on specified block device
  (`UUID=...`, `LABEL=...`, or `/dev/...`) and save dump/report to `rd.crash.dir=`. If set to
  `auto` or omitted with flag enabled, automatically searches for partitions labeled `CRASH` or `KDUMP`.
* `rd.crash.raw=<device>`: Stream dump/report directly to a raw block device without a filesystem.
* `rd.crash.dir=`: Subdirectory where dump files are stored (default: `EFI/crash` for EFI, `var/crash` for disk).

### Boot & Kernel Inspection
* `rd.crash.boot=`: Block device containing the `vmlinux` kernel image
  for `crash` triage (supports `UUID=...`, `LABEL=...`, `/dev/...`; falls back to `root=`).
* `rd.crash.vmlinux=`: Relative path to `vmlinux` on the mounted boot filesystem.
* `rd.crash.boot-timeout=`: Maximum seconds to wait for boot device or virtio port to appear (default: 10).

### Exit Policy
* `rd.crash.exit=`: Action to perform when capture finishes:
  * `reboot` (default): Reboot system after delay.
  * `shell`: Drop into an interactive shell on `/dev/console`.
  * `poweroff` (or `halt`): Power off the machine.
* `rd.crash.reboot-delay=`: Seconds to wait before rebooting (default: 5).

## Report Formats and Host Extraction

Depending on `rd.crash.report=` and the chosen target destination, data is packaged into structured, self-identifying formats with well-defined signatures at sector 0:

| Scenario / Destination | Content | Container Format | Magic / Signature (Offset 0) | Host Reassembly & Extraction |
|---|---|---|---|---|
| **`rd.crash.report=dump`** on raw disk (`rd.crash.raw`) | `meta.txt`, `dmesg.txt.gz`, `triage.txt.gz` | **CPIO archive** (SVR4 `newc`) | `"070701"` | `cpio -idmv < /dev/<raw_dev>` or `tar -xf /dev/<raw_dev>` |
| **`rd.crash.report=dump`** via virtio (`rd.crash.virtio`) | `meta.txt`, `dmesg.txt.gz`, `triage.txt.gz` | **CPIO archive** (SVR4 `newc`) | `"070701"` | `socat UNIX-LISTEN:/tmp/crash.sock,reuseaddr - \| cpio -idmv` |
| **`rd.crash.report=dump`** on filesystem (`disk`, `efi`) | `crash-*.cpio` (`meta.txt`, `dmesg.txt.gz`, `triage.txt.gz`) | **CPIO archive** (SVR4 `newc`) | `"070701"` | `cpio -idmv < /path/to/crash-*.cpio` or `tar -xf /path/to/crash-*.cpio` |
| **`rd.crash.report=vmcore`** on raw disk (`rd.crash.raw`) | Full compressed kernel memory dump | **KDUMP format** (`makedumpfile -c`) | `"KDUMP   "` | `crash vmlinux /dev/<raw_dev>` or `makedumpfile -R vmcore /dev/<raw_dev>` |
| **`rd.crash.report=vmcore`** via virtio (`rd.crash.virtio`) | Full sequential dump stream | **Flattened KDUMP** (`makedumpfile -F`) | `"makedumpfile"` | `socat UNIX-LISTEN:/tmp/crash.sock,reuseaddr - \| makedumpfile -R vmcore -` |
| **`rd.crash.report=vmcore`** on filesystem (`disk`, `efi`) | `vmcore-*.dump` | **KDUMP format** (`makedumpfile -c`) | `"KDUMP   "` | `crash vmlinux /path/to/vmcore-*.dump` |

### Why CPIO for `report=dump`
* **Sector 0 Signature:** The `"070701"` magic starts at byte offset 0. `file -s /dev/<raw_dev>` immediately identifies the partition as `ASCII cpio archive (SVR4 with no CRC)`.
* **Clean End of Data:** The archive terminates with a `TRAILER!!!` record. Extraction tools (`cpio -idmv`, `tar -xf`, `bsdtar`) stop immediately upon reaching the trailer, ignoring any trailing zeroes or unallocated space on raw disk partitions.
* **Separation of Files:** Keeps individual log components (`dmesg`, triage analysis, kernel metadata) separate and timestamped without risking stream corruption.

### Why Native KDUMP for `report=vmcore`
* **Direct Inspection:** Because raw disk partitions support `lseek`, `makedumpfile -c` writes the standard seekable KDUMP container directly to the partition with the `"KDUMP   "` signature at offset 0. The `crash` debugger and `makedumpfile` can open `/dev/<raw_dev>` directly without requiring an intermediate extraction step.
* **Memory Safety:** Full memory dumps can exceed several gigabytes, making in-RAM archive buffering impossible within small `crashkernel=` reservations. On non-seekable streams like virtio-serial, `makedumpfile -F` writes its sequential flattened format directly without intermediate buffering.

## Build Parameters

* `CRASH_ENABLE_LIVE`: Set to `1` (default) to include the `crash` debugger; set to `0`
  to exclude it and build an ultra-lightweight dump-only initramfs.
* `CRASH_CHECK_SIZE`: Validation policy for capture initramfs size (`warn`, `error`, or `none`; default: `warn`).
* `CRASH_RESERVE_SIZE`: Expected crashkernel memory reservation (default: `256M`). If unset or default, automatically detected from `/sys/kernel/kexec_crash_size`, `/proc/iomem`, or `/proc/cmdline`.
* `CRASH_MAX_ROOTFS_RATIO`: Maximum allowed ratio of unpacked rootfs size to crashkernel reserve percentage (default: `35`%).
* `CRASH_MAX_IMAGE_RATIO`: Maximum allowed ratio of packed initramfs image size to crashkernel reserve percentage (default: `20`%).
* `CRASH_MAX_ROOTFS_SIZE`: Explicit limit for unpacked rootfs size (e.g. `90M`). Overrides ratio calculation.
* `CRASH_MAX_IMAGE_SIZE`: Explicit limit for packed image size (e.g. `50M`). Overrides ratio calculation.

* `CRASH_PROG`: Name or path of the crash binary (default: `crash`).
* `CRASH_DUMP_PROGS_PATTERNS`: Binary patterns for dump tools (`*/makedumpfile */vmcore-dmesg */findfs`).
* `CRASH_EXTRA_PROGS`: Optional explicit paths to additional binaries.

## Crash Lifecycle Hooks (`/lib/initrd/crash/crash.d/`)

Administrators can customize or extend crash dump collection by packaging hook
scripts into `/lib/initrd/crash/crash.d/[0-9][0-9]-*.sh`. Hooks execute in
alphabetical order and receive the active lifecycle phase as their first argument:

* **`$1`**: Lifecycle phase (`prepare`, `report`, `write`, `finish`).
* **`$2`**: Crash runtime directory `$crash_dir` (default: `/run/crash`).

Keeping all lifecycle phases within a single hook script avoids fragmenting complex
logic (such as mounting, uploading, and unmounting remote storage) across multiple
disconnected files.

### Lifecycle Phases

The capture service invokes each script in `/lib/initrd/crash/crash.d/` across four phases:

| Phase | Invocation & Arguments | Purpose |
|---|---|---|
| **`prepare`** | `"$hook" prepare "$crash_dir"` | Bring up network interfaces (DHCP/static), mount network shares (NFS, CIFS), initialize hardware or watchdog. |
| **`report`** | `"$hook" report "$crash_dir"` | Inject additional diagnostic files into the CPIO stream on the fly via `cpio-stream add`. Runs inside `cpio_report` before archive termination. Diagnostics and logs must be printed to **`stderr`** (`>&2`) to avoid corrupting the stream. |
| **`write`** | `cpio_report \| "$hook" write "$crash_dir"` | Custom target storage. The hook reads the CPIO archive directly from **`stdin`** and writes or streams it to remote destinations without RAM buffering. Exiting with code `0` marks the target handled. |
| **`finish`** | `"$hook" finish "$crash_dir"` | Teardown and notification: unmount network shares, send completion webhooks/alerts, execute `sync`. |

### Packaging Hooks

Hooks can be packaged into the initramfs using standard `PUT_FILES`:

```makefile
PUT_FILES += /lib/initrd/crash/crash.d/50-nfs.sh
```

### Examples

#### 1. Complete NFS Storage Lifecycle (`crash.d/50-nfs.sh`)

```bash
#!/bin/sh
phase="$1"
crash_dir="$2"

case "$phase" in
	prepare)
		mkdir -p /mnt/nfs
		mount -t nfs -o nolock 192.168.1.100:/srv/crash /mnt/nfs
		;;
	report)
		# Add host inventory into the CPIO archive (diagnostic messages to stderr)
		[ ! -f /etc/machine-id ] || cpio-stream add "machine-id" /etc/machine-id
		;;
	write)
		# Stream CPIO report from stdin directly to NFS without RAM buffering:
		mkdir -p /mnt/nfs/dumps
		cat > "/mnt/nfs/dumps/crash-$(date +%Y%m%d-%H%M%S).cpio"

		# If full memory dump is requested:
		if [ "${RD_CRASH_REPORT:-dump}" = "vmcore" ]; then
			makedumpfile -c -d 31 "$VMCORE" "/mnt/nfs/dumps/vmcore-$(date +%Y%m%d-%H%M%S).dump" || :
		fi
		;;
	finish)
		sync
		umount /mnt/nfs 2>/dev/null || :
		;;
esac
```

#### 2. Streaming Report via HTTP POST (`crash.d/60-http-upload.sh`)

```bash
#!/bin/sh
[ "$1" = "write" ] || exit 0
crash_dir="$2"

# Upload CPIO report stream directly to an HTTP server via curl:
curl -s -S -T - "http://crash-collector.corp.internal/upload/$(uname -n)-$(date +%s).cpio"
```

#### 3. Streaming Report to Remote Socket (`crash.d/60-netcat.sh`)

```bash
#!/bin/sh
[ "$1" = "write" ] || exit 0
crash_dir="$2"

# Stream CPIO archive directly across the network to a listening receiver:
nc -w 10 192.168.1.50 9999
```

#### 4. Encrypted Storage on Mounted Share (`crash.d/70-gpg-encrypt.sh`)

```bash
#!/bin/sh
[ "$1" = "write" ] || exit 0
crash_dir="$2"

# Encrypt CPIO archive on the fly with recipient key before saving:
gpg --batch --recipient admin@corp.internal --encrypt | \
	cat > "/mnt/nfs/dumps/crash-$(date +%Y%m%d-%H%M%S).cpio.gpg"
```
