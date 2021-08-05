# Feature: bootloader

The feature adds the ability to create an image that can act as a bootloader.
The image looks for the partition that is mounted at /boot or the partition
where the /boot directory exists.

# Config files

* `/boot/bootloacer.conf` -- the configuration file lists the kernels and initrd
   for later boot.

