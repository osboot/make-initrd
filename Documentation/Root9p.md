# 9P as root filesystem

It is possible to run a whole virtualized guest system entirely on top of QEMU's
9p pass-through filesystem such that all guest system's files are directly
visible inside a subdirectory on the host system and therefore directly
accessible by both sides.

In this case, it makes no sense to wait for some kind of linux kernel event.

Example of cmdline to mount 9p as a root filesystem:
```
root=SOURCE=rootfs rootflags=rw,trans=virtio,version=9p2000.L rootfstype=9p
```

See also [QEMU documentation](https://wiki.qemu.org/Documentation/9p_root_fs)
how to setup 9p root fs.
