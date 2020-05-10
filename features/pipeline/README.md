# Feature: pipeline

The feature calls the pipe elements one by one and passes them the results of
the previous elements.

## Pipe elements

- `getimage` receives and mounts the remote image.
- `mountfs` mounts file from the previous pipe element.
- `overlayfs` combines one or more pipe elements using overlayfs.
- `waitdev` waits for the local device to appear.
- `rootfs` uses the result of the previous element as a root.

## Boot parameters

- `pipeline=name[,name1][,name2]` - the main parameter that determines the order
  in which pipe elements are called.
- `getimage` specifies an URL to fetch and mount.
- `mountfs` specifies a file to mount.
- `overlayfs` defines a list of elements to be combined.
- `waitdev` describes the local device to wait. The format of this parameter is
   the same as `root=`.

The separator between the elements is a comma (`,`).

The parameters can be specified more than once depending on how many times
a corresponding element is mentioned in the `pipeline`.

## Example

Cmdline: root=pipeline pipeline=getimage,mountfs,overlayfs,rootfs getimage=http://ftp.altlinux.org/pub/people/mike/iso/misc/vi-20140918-i586.iso mountfs=rescue

Following these parameters, the pipeline downloads the vi-20140918-i586.iso
image, mount it as a loop, make it writable using overlayfs and will try to
boot from it.
