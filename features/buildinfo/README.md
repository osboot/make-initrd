# Feature: buildinfo

Feature saves information about the created initramfs image.

## Files

- The features that were used to build the image: `/var/lib/initrd/$(KERNEL).$(IMAGE_SUFFIX)/features`.
- The files that were copied to the image: `/var/lib/initrd/$(KERNEL).$(IMAGE_SUFFIX)/files`.
