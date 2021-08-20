# Feature: bootloader

The feature adds the ability to create an image that can act as a bootloader.
The image looks for the partition that is mounted at /boot or the partition
where the /boot directory exists.

## Config files

* `/boot/bootloader.conf` -- the configuration file lists the kernels and initrd
   for later boot.

## Utils
There is some utils to create config files, linux bootloader initrd image and kernel. You may find it [here](https://github.com/osboot/make-initrd-bootloader)

## bootloader.conf example
[global]
        timeout = 10
        cmdline = ro root=UUID=d82b8b8d-389c-43e9-97de-487e025d1a01

[boot "Default"]
        kernel = /boot/vmlinuz
        initrd = /boot/initrd.img

[boot "Kernel (5.11.0-25-generic)"]
        kernel = /boot/vmlinuz-5.11.0-25-generic
        initrd = /boot/initrd.img-5.11.0-25-generic

[boot "Kernel (5.4.0-42-generic)"]
        kernel = /boot/vmlinuz-5.4.0-42-generic
        initrd = /boot/initrd.img-5.4.0-42-generic
