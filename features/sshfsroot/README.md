# Feature: sshfsroot

Feature adds the ability to mount the root using SSH (more precisely, the SFTP
subsystem). Most SSH servers support and enable this SFTP access by default, so
SSHFS is very simple to use

## Parameters

- **SSHFS_KNOWN_HOSTS** -- A `known_hosts` file that the ssh client must use (optional).
- **SSHFS_CONFIG** -- A `.ssh/config` file for ssh client (optional).
- **SSHFS_KEY** -- A ssh key to use for sshfs connection (optional).
- **SSHFS_KEYS** -- A list of ssh keys that need to be put to the initramfs user (optional).

## Boot parameters

To mount root over sshfs, parameter `root=/dev/sshfs` should be specified.

- `sshfsroot=[user@]<server>:<root-dir>` -- mount remote filesystem share from <server>:/<root-dir>
- `sshfsopts=options` -- options that are passed to sshfs.
