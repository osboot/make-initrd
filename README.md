# make-initrd

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

- [GNU make](http://www.gnu.org/software/make/) is used to generate an
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
- [json-c](https://github.com/json-c/json-c) is needed to get dependencies from
  [ELF DLOPEN Medatata](https://github.com/systemd/systemd/blob/main/docs/ELF_DLOPEN_METADATA.md).
  The systemd-256 switched to dynamic loading of libraries that the authors
  consider optional.
- Compression libraries are required for the initramfs image parsing utilities:
  [zlib](https://zlib.net), [bzip2](https://www.sourceware.org/bzip2/),
  [xz](http://tukaani.org/xz/), [zstd](https://facebook.github.io/zstd/).
- [scdoc](https://git.sr.ht/~sircmpwn/scdoc) is used to generate man-pages.

## Support
make-initrd aims to be highly portable, though it has only been tested on a
limited number of systems.

- Operating Systems: [ALT Linux](https://en.altlinux.org/), [Fedora](https://fedoraproject.org/),
  [Gentoo](https://www.gentoo.org/), [Ubuntu](https://ubuntu.com/).
- C Library: [GNU libc](https://www.gnu.org/software/libc/), [musl libc](https://musl.libc.org/).

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

