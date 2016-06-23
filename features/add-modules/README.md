# Feature: add-modules

Feature adds modules to initramfs.

## Parameters

- **MODULES_ADD** -- The parameter contains the names of modules to be added to the image. Thay can be loaded by udev if needed when it starts.
- **MODULES_PRELOAD** -- The parameter contains the names of modules to be added to the image and unconditionally loaded before udev start.
- **MODULES_LOAD** -- The parameter contains the names of modules to be added to the image and unconditionally loaded after udev start.
- **RESCUE_MODULES** -- The names of modules to be loaded if emergency shell called.
- **BLACKLIST_MODULES** - The names of modules that should NOT present in the image.
