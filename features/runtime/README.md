# Feature: Runtime

The directory contains files that will always be packed into initramfs. Feature
provides init, services, and other basic components.

- **data** -- directory contains static files that will be copied as is.
- **src** -- directory contains the source code of the utilities that will be
  compiled and added to the initramfs.

## See also

- The article about [runtime](../../Documentation/BootInitramfs.md).
- The article about [boot parameters](../../Documentation/BootParameters.md).
- About [initramfs services](../../Documentation/InitramfsServices.md).
