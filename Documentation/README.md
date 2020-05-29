# Overview

The only propose of initramfs is getting the root filesystem mounted.

make-initrd is a program for creation initramfs. Logically, the program can be
divided into three parts:

- Image generator;
- Automatic configuration detector;
- A set of programs run inside the image.

If you have problems with `make-initrd` or think you've found a bug, please
report it to the developers; we cannot promise to do anything but we might well
want to fix it.

Patches are welcome.

## Requires

- make-initrd uses [GNU make](http://www.gnu.org/software/make/) to establish
  and properly maintain the relationship between modules.

- Most scripts use [libshell](https://github.com/legionus/libshell), a set of
  functions frequently used in shell scripts.

- [kinit-utils](https://github.com/legionus/kinit-utils) is a set of
  tools from [klibc](https://kernel.org/pub/linux/libs/klibc) ported to glibc.
  These tools have broad enough functionality and small size. These utilities
  are used by some features (setup network, NFS root, raid autodetect).

## See also

- The article about [main idea of determining the root](HowItWorks.md).
- The article about [runtime](BootRuntime.md).
- Feature [list](Features.md).
