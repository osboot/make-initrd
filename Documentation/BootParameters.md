# Boot parameters

The following parameters can be passed via kernel command line.

#### Common parameteres

- `init=<path>` specifies the path to the init programm to be started after the initramfs has finished.
- `panic` enables/disables the receiving console in emergency situations.
- `rescue-modules` specifies the list of modules required to open emergency console.
- `stop=<comma-separated list>` specifies the list of stages to where it's necessary to open emergency console.
- `rdshell` opens emergency console after all services.
- `rdlog=console` all messages from services will be printed to /dev/console.
- `debug` shows more messages.
- `quiet` tries to be more quiet.
- `rd-preload-modules` enables/disables loading a static list of modules before udev service.

Parameters responsible for mounting the root file system:

- `root=<DEVICE-SPEC>` specifies the block device to use as the root filesystem
  (see [device spec](DeviceSpec.md)).
- `rootflags=<options>` specifies additional mount options.
- `rootfstype=<type>` specifies root filesystem type.
- `rootdelay=<seconds>` specifies how long to wait for the root filesystem.
- `rootonly` ignore all mountpoints except root filesystem.
- `ro` mount root filesystem read-only.
- `rw` mount root filesystem read-write.

These parameters may not be specified if booting occurs on the same system and
with the same configuration where the initramfs image was created. The `root=`,
`rootfstype=`, `rootflags=` are stored internally when generating the image.
If `root=` option is found in kernel command line, then all stored values are
discarded.

#### Resume

- `resume=<path|uuid|label>` resume from a swap partition.
- `resume_offset` offset in swap partition.
- `noresume` not resume and continue boot.
