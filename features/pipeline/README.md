# Feature: pipeline

The feature calls the pipe elements one by one and passes them the results of
the previous elements.

This feature is intended for boot scenarios not related to simple disk
initialization. Normal disk loading can also be done using pipeline (see
examples).

To activate a pipeline, you must specify it as `root=pipeline` in the boot
cmdline. The sequence of steps is specified in the `pipeline=...` parameter.

## Pipe elements

- `getimage` receives and mounts the remote image.
- `mountfs` mounts file from the previous pipe element.
- `overlayfs` combines one or more pipe elements using overlayfs.
- `waitdev` waits for the local device to appear.
- `rootfs` uses the result of the previous element as a root.
- `wait-resume` is waiting for the resume attempt to complete. This step is
  necessary to avoid race conditions between disk initialization for resume and
  pipeline.
- `ping` allows to wait until the network address starts pinging.

## Boot parameters

- `pipeline=NAME[,NAME1][,NAME2]` - the main parameter that determines the order
  in which pipe elements are called.
- `getimage=URL` specifies an URL to fetch and mount. If a file from a local
  directory is to be retrieved, the `file://` scheme must be used.
- `mountfs=FILE` specifies a file to mount.
- `mountfs-opts=MOUNT-OPTIONS` comma (`,`) separated mount options.
- `overlayfs=LAYERS` defines a list of elements to be combined.
- `waitdev=DEVSPEC` describes the local device to wait. The format of this
  parameter is the same as `root=`.
- `ping=[OPTIONS:][ADDRESS]` The comma (`,`) separated options include: `v4`,
   `v6` - determines the ip version; `iter=NUMBER` specifies the number of
   attempts to ping the address; `waitfor` if specified, the wait will be
   forever (this is the default behavior). The address can be an ip address or a
   hostname, or `%gateway`, which stands for the default gateway address.
   The default ADDRESS is `%gateway`.
- `pipe-retry=NUMBER` determines how many times a certain step can fail
  (single parameter).
- `pipe-verify-sign=NAME[,NAME1][,NAME2]` defines a list of steps where
  signature verification should be enabled (single parameter).

The separator between the elements is a comma (`,`).

The parameters can be specified more than once depending on how many times
a corresponding element is mentioned in the `pipeline`.

## Example

### Simple boot.

Cmdline: root=pipeline pipeline=waitdev,mountfs,rootfs waitdev=LABEL=MYROOT mountfs=dev

Using these parameters, the pipeline will wait for a device with the `"MYROOT"`
label, mount it, and boot from it.

### Network boot

Cmdline: root=pipeline pipeline=getimage,mountfs,overlayfs,rootfs getimage=http://ftp.altlinux.org/pub/people/mike/iso/misc/vi-20140918-i586.iso mountfs=rescue

Following these parameters, the pipeline downloads the vi-20140918-i586.iso
image, mount it as a loop, make it writable using overlayfs and will try to
boot from it.
