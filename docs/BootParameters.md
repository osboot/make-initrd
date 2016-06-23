# Boot parameters

The following parameters can be passed via kernel command line.

#### Common parameteres

- `init=<path>` specifies the path to the init programm to be started after the initramfs has finished.
- `panic` enables/disables the receiving console in emergency situations.
- `rescue-modules` specifies the list of modules required to open emergency console.
- `stop=<comma-separated list>` specifies the list of stages to where it's necessary to open emergency console.
- `rdshell` opens emergency console after all services.
- `debug` shows more messages.
- `quiet` tries to be more quiet.

Parameters responsible for mounting the root file system:

- `root=<uuid|label|path|major:minor>` specifies the block device to use as the root filesystem.
- `rootdelay=<seconds>` specifies how long to wait for the root filesystem.
- `rootonly` ignore all mountpoints except root filesystem.
- `rootflags` specifies additional mount options.
- `rootfstype=<type>` specifies root filesystem type.
- `ro` mount root filesystem read-only.
- `rw` mount root filesystem read-write.

#### Resume

- `resume=<path|uuid|label>` resume from a swap partition.
- `resume_offset` offset in swap partition.
- `noresume` not resume and continue boot.

#### Network parameters

- `ip=<client-ip>:<server-ip>:<gw-ip>:<netmask>:<hostname>:<device>:<proto>` or `ip=<device>:<proto>` describes the configuration of network interfaces;
- `iptimeout=<seconds>` describes the time for which the network interface must be initialized.

#### NFS root

- `nfsroot=[<server-ip>:]<root-dir>[,<nfs-options>]` mount nfs share from <server-ip>:/<root-dir>
- `nfsopts`

#### LUKS

- `no-luks` disables crypto LUKS detection.
- `luks-dev`
- `luks-key`

#### plymouth

- `vga`
- `splash`
- `nosplash`
