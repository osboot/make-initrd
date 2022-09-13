# Runtime

If want to know how it works in general please read [this document](HowInitramfsWorks.md).
It describes the basic idea of determining the root.

## Boot order

- kernel calls `/init`. In the initramfs `/init` is the simplest script which
  saves the environment variables and starts/stops [services](InitramfsServices.md)
  on runlevel 3.
- When the root file system found init goes to runlevel 2. Last service on
  runlevel 2 asks init to restart. When the SysV init receives request to reboot
  himself uses predefined path (`/sbin/init`).
- `/sbin/init` is another script which removes initramfs, restore kernel
  environment and runs real init.

## Services

The features may install one or more initialization scripts (or init scripts).
An init script shall be installed in `/etc/init.d` (which may be a symbolic link
to another location). The initramfs uses SysV-style [init scripts](InitramfsServices.md).

## Run Levels

Basically run levels are specified for use by the `Default-Start` and
`Default-Stop` actions defined in the [init scripts](InitramfsServices.md).
The LSB standard defines them as:

- 0 halt;
- 1 single user mode;
- 2 multiuser with no network services exported;
- 3 normal/full multiuser;
- 4 reserved for local use, default is normal/full multiuser;
- 5 multiuser with a display manager or equivalent;
- 6 reboot;

but implementations are not required to provide these exact run levels or give
them the meanings described here. The initramfs uses following run levels:

- ...
- 2 is used to switch to real system. The switch to this level occurs only after
  the rootfs is found and mounted.
- 3 default initramfs runtime.
- 9 exec real system in the `/root`.
- ...

## See also

- The article about [boot parameters](BootParameters.md).
- About [initramfs services](InitramfsServices.md).
