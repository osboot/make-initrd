# kernel-install plugin

`50-make-initrd.install` lets [kernel-install(8)](https://www.freedesktop.org/software/systemd/man/latest/kernel-install.html)
build the initramfs with make-initrd when `kernel-install add` is called
without an initrd argument. The image is written to the staging area and
picked up by the layout plugins (`90-loaderentry`, `90-uki-copy`).

The plugin builds an initramfs when `initrd_generator` is unset or set to
`make-initrd`. It skips this step when the kernel-install configuration
selects another generator or sets `initrd_generator=none`.

With `layout=uki` and `uki_generator=make-initrd` the plugin builds a
unified kernel image with the `uki` feature instead, regardless of
`initrd_generator`. The `uki` feature always builds its own initramfs, so
the image is built even when an initrd is passed on the command line; that
initrd is not embedded, and the plugin warns about it.
