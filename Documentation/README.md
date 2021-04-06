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

## Articles

- The article about [main idea of determining the root](HowItWorks.md).
- The article about [runtime](BootRuntime.md).
- Article to learn how to write [new features](NewFeature.md).
- Incomplete feature [list](Features.md). To learn more about features, see the
  documentation in the feature directory.
