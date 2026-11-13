# Feature: device-daemon-mdev

This feature provides lightweight device node population using `mdev`
from BusyBox, suitable for resource-constrained and capture environments.
It also populates persistent disk symlinks in `/dev/disk/by-{uuid,label,partuuid,partlabel,id}`,
Device-Mapper nodes in `/dev/mapper/<name>`, and `/dev/cdrom` via `/etc/mdev.conf` and `blkid`.
