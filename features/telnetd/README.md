# Feature: telnetd

Feature adds a telnet server to initramfs.

It installs the telnet service runtime files and starts the service during
early boot. The feature requires the `network` feature.

## Configuration

- `/etc/telnetd/telnetd.conf` can be used to pass additional options to
  `telnetd`.
