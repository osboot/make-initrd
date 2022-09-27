# make-initrd [![Integration Tests](https://github.com/osboot/make-initrd/actions/workflows/integration.yml/badge.svg)](https://github.com/osboot/make-initrd/actions/workflows/integration.yml)

make-initrd is a new, uevent-driven initramfs infrastructure based around udev.

The make-initrd is intended for creating initramfs images. The project consists
of two main parts:
- Utilities for initramfs image creation (this includes determining the kernel
  modules required for the current hardware, adding required utilities, etc).
- Utilities that run inside the initramfs image at boot time.

The project has a modular structure. Its main idea is that to create the image you need
(in the simplest case) to specify a list of characteristics (features) of the future image.
The order of features does not matter. The user is not required to know how and in what order
will apply stage.

## Documentation

Documentation about the project is located in the [Documentation directory](Documentation/)
or on the [wiki](https://github.com/osboot/make-initrd/wiki) where the information
is collected in one place.

## Build Requires

To build a initramfs, you have to install the following software packages:

- [gnu make](http://www.gnu.org/software/make/) is used to generate an
  initramfs image. This is a small but very powerful utility.
- [bash](https://www.gnu.org/software/bash/) is needed for all scripts in the
  project. This allows for increased portability from one distribution to
  another.
- [busybox](https://busybox.net/) is used in the initramfs as runtime (*submodule*).
  This allows for increased portability from one distribution to another.
- [libshell](https://github.com/legionus/libshell) is used both when generating
  the image and in the initramfs (*submodule*).
- Utilities from [coreutils](https://www.gnu.org/software/coreutils/) are used
  in the initramfs image generation scripts.
- [kmod](https://git.kernel.org/pub/scm/utils/kernel/kmod/kmod.git) is needed
  to calculate dependencies and load kernel modules in the initramfs.
- [elfutils](https://sourceware.org/elfutils/) is optional but highly desirable
  for proper operation. This library is used to calculate the dependencies of
  binary utilities that are copied into the initramfs image.
- Compression libraries are required for the initramfs image parsing utilities.
  - [zlib](https://zlib.net)
  - [bzip2](https://www.sourceware.org/bzip2/)
  - [xz](http://tukaani.org/xz/)
  - [zstd](https://facebook.github.io/zstd/)
- [help2man](https://www.gnu.org/software/help2man/) is used to generate man-pages.

## Build

```bash
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

## License

Make-initrd is licensed under the GNU General Public License (GPL), version 3.

