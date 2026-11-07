# Feature: shutdown

This feature adds support for cleanly unmounting and tearing down storage stacks
during system shutdown.

When a system running systemd is shut down or rebooted, systemd performs a
`pivot_root` into `/run/initramfs` and executes `/shutdown` (the exitrd
mechanism). This allows the real root filesystem (`/oldroot`) to be safely
unmounted, and underlying block devices (LUKS containers, LVM volume groups, MD
RAID arrays, multipath) to be cleanly deactivated.

## Shutdown hooks

Any active feature can provide shutdown hooks by adding files to its `shutdown/`
directory. The contents of all `features/*/shutdown/` directories from active
features are automatically included into the initramfs.

All hooks are invoked with a consistent signature:
```
"$hook" "$hook_type" "$action" "$target" [metadata...]
```

### Exit codes convention

Hooks indicate their handling status via standard exit codes:
- `0`: Device was handled and successfully dismantled (progress made).
- `2`: Device was skipped / not applicable to this hook.
- `1` (or other non-zero): Error occurred during dismantling.

In `post-umount.d`, `/shutdown` repeats leaf teardown iterations as long as at
least one hook returns `0` (making progress).

### Hook stages

Shutdown hooks are organized into the following stages:

1. **`lib/initrd/shutdown/pre-umount.d/`**
   - Executed before unmounting begins.
   - Arguments: `"$hook_type"` `"$action"` `"/oldroot"`
   - Use cases: early storage preparations (e.g. `mdmon --takeover --all`),
     flushing caches, diagnostics.

2. **`lib/initrd/shutdown/umount.d/`**
   - Executed for each mountpoint right before calling `umount -d`.
   - Arguments: `"$hook_type"` `"$action"` `"$mountpoint"` `"$fstype"`
   - Use cases: filesystem-specific teardown (flushing network filesystems,
     stopping filesystem-specific daemons).

3. **`lib/initrd/shutdown/post-umount.d/`**
   - Executed during topological block device teardown for each leaf device (no
     entries in `holders/`).
   - Arguments: `"$hook_type"` `"$action"` `"$sysfs_path"` `"$major"`
   - Use cases: modular deactivation of specific block layers (`10-loop.sh`,
     `50-dm.sh`, `60-md.sh`, etc.).

4. **`lib/initrd/shutdown/shutdown.d/`**
   - Executed once at the very end after all mountpoints and block devices have
     been torn down, immediately before `sync` and reboot/poweroff.
   - Arguments: `"$hook_type"` `"$action"`

## Auto-detection (guess)

This feature provides `guess/common`:
When generating a new initramfs, `guess/common` queries `initrd-find` to locate
the previous or current kernel's initramfs image. If that image contains
`/shutdown`, `features/shutdown` is automatically guessed (`guess_feature shutdown`)
for the new image.

## Systemd integration

Host system components handle exitrd restoration:
- `initrd-find` (installed into `/usr/sbin/` from `utils/`):
  utility used to locate the active or matching initramfs image across BLS Type #1
  configurations, BLS directory trees, BLS Type #2 UKIs, `/proc/cmdline`,
  `/lib/modules/$kver/initrd`, and standard boot directories.
- `make-initrd-shutdown-restore` (installed into `/usr/sbin/` from `utils/`):
  utility executed upon shutdown to unpack the initramfs into `/run/initramfs`.
  It locates the initramfs via `initrd-find` and only unpacks if `/shutdown`
  is present in the archive (meaning feature `shutdown` was included).
- `make-initrd-shutdown.service` (in `system/`): systemd unit triggering
  `make-initrd-shutdown-restore` on shutdown.
