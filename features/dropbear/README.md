# Feature: dropbear

Feature adds the Dropbear SSH server to initramfs.

It installs the Dropbear runtime files and starts the service during early boot.
The feature requires the `network` and `syslog` features.

## Configuration

- `/etc/dropbear/dropbear.conf` can be used to pass additional Dropbear options.

## Notes

- Host keys are generated automatically in initramfs if they are missing.
- The feature adds the `dropbear`, `dropbearkey`, and `dropbearconvert`
  programs to the image.
