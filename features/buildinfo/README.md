# Feature: buildinfo

Feature saves information about the created initramfs image. This information
contains such information as the features used and the list of packed files.

## Parameters

- **BUILDINFODIR** -- The directory where the buildinfo will be saved.

## Files

- The features that were used to build the image: `/var/lib/initrd/$(KERNEL).$(IMAGE_SUFFIX)/features`.
- The files that were copied to the image: `/var/lib/initrd/$(KERNEL).$(IMAGE_SUFFIX)/files`.
