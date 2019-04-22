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
- `rd-preload-modules` enables/disables loading a static list of modules before udev service.

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

[See network feature](../features/network/README.md)

#### NFS root

[See nfsroot feature](../features/nfsroot/README.md)

#### LUKS

[See luks feature](../features/luks/README.md)

#### plymouth

[See plymouth feature](../features/plymouth/README.md)
