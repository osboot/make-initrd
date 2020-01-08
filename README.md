# make-initrd

[![Build Status](https://travis-ci.org/legionus/make-initrd.svg?branch=master)](https://travis-ci.org/legionus/make-initrd)

make-initrd is a new, uevent-driven initramfs infrastructure based around udev.

The make-initrd is intended for creating initramfs images. The project consists of two main parts:
- utility for creating initramfs;
- utilities running inside the image at boot time.

Utility of imaging has a modular structure. Its main idea is that to create the image you need
(in the simplest case) to specify a list of characteristics (features) of the future image.
The order of features does not matter. The user is not required to know how and in what order
will apply stage.
