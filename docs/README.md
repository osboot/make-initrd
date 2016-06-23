# Overview

The only propose of initramfs is getting the root filesystem mounted.

make-initrd is a program for creation initramfs. Logically, the program can be divided into three parts:

- Image generator;
- Automatic configuration detector;
- A set of programs run inside the image.

If you have problems with 'make-initrd' or think you've found a bug, please report it to the developers; we cannot promise to do anything but we might well want to fix it.
Patches are welcome.

## Requires

- make-initrd uses GNU make to establish and properly maintain the relationship between modules.

- Most scripts use [libshell](https://github.com/legionus/libshell), a set of functions frequently used in shell scripts.

- [kinit-utils](http://git.altlinux.org/gears/k/kinit-utils.git) is a set of tools from klibc ported to glibc. These tools have broad enough functionality and small size.
These utilities are used by some features (setup network, NFS root, raid autodetect).

- [SysV init](http://savannah.nongnu.org/projects/sysvinit) used inside initramfs. It was chosen because it is small, simple and able to execute instead of self any other program.
