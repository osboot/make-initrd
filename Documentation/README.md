# Overview

The only propose of initramfs is getting the root filesystem mounted.

make-initrd is a program for creation initramfs. Logically, the program can be
divided into three parts:

- Automatic [configuration detector](GuessConfiguration.md) which complements
  the user configuration;
- Image generator which creates an initramfs image using the user's augmented
  configuration;
- A set of [programs](HowInitramfsWorks.md) run inside the initramfs image.

If you have problems with `make-initrd` or think you've found a bug, please
report it to the developers; we cannot promise to do anything but we might well
want to fix it.

Patches are welcome.
