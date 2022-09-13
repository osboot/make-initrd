# Feature: guestfs

This feature is used to replace libguestfs [appliance](https://github.com/libguestfs/libguestfs/tree/master/appliance)
which is usually created with [supermin](https://github.com/libguestfs/supermin).
This appliance is meant to run inside a virtual machine.

The libguestfs utilities are designed to work with virtual machine images. The
libguestfs utilities do not just mount the virtual machine image, but start the
qemu virtual machine, specifying the kernel and initrd. Inside the virtual
machine, a special guestfsd daemon is launched in the initrd, which accepts
commands from outside. So, commands from outside are sent to the guestfsd
daemon via the virtio-serial channel (`/dev/virtio-ports/org.libguestfs.channel.0`)
and executed inside the virtual machine.

The guestfs initrd image is usually created with the command:

```bash
$ make-initrd --no-checks --config=/etc/initrd.mk.d/guestfs.mk.example --kernel=$KVER
```
