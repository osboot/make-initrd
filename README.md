# make-initrd [![Integration Tests](https://github.com/osboot/make-initrd/actions/workflows/integration.yml/badge.svg)](https://github.com/osboot/make-initrd/actions/workflows/integration.yml)

make-initrd is a new, uevent-driven initramfs infrastructure based around udev.

The make-initrd is intended for creating initramfs images. The project consists of two main parts:
- utility for creating initramfs;
- utilities running inside the image at boot time.

Utility of imaging has a modular structure. Its main idea is that to create the image you need
(in the simplest case) to specify a list of characteristics (features) of the future image.
The order of features does not matter. The user is not required to know how and in what order
will apply stage.

## Build requires

To build a initramfs, you have to install the following software packages:

- [coreutils](https://www.gnu.org/software/coreutils/)
- [gnu make](http://www.gnu.org/software/make/)
- [kmod](https://git.kernel.org/pub/scm/utils/kernel/kmod/kmod.git)
- [help2man](https://www.gnu.org/software/help2man/)
- [elfutils](https://sourceware.org/elfutils/)

Optional:

- [busybox](https://busybox.net/)
- [libshell](https://github.com/legionus/libshell)
- [zlib](https://zlib.net)
- [bzip2](https://www.sourceware.org/bzip2/)
- [xz](http://tukaani.org/xz/)

## Build

```
$ ./autogen.sh
$ ./configure
$ make
$ make install DESTDIR=/path/to/somewhere
```

If you want to use it in-tree:
```
$ ./configure --enable-local-build
$ make
```

## Documentation

Documentation about the project is located in the [Documentation directory](Documentation/).

## License

Make-initrd is licensed under the GNU General Public License (GPL), version 3.

