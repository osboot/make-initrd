# Feature: uki

Unified kernel image (UKI) is a single executable which can be booted directly
from UEFI firmware, or automatically sourced by boot-loaders with little or no
configuration.

See https://uapi-group.org/specifications/specs/unified_kernel_image/

## Parameters

- **UKI_FEATURES** -- Variable contains a list of features that will be
  added to the UEFI image.
- **UKI_SPLASH_FILE** -- Variable contains a filename which will be used as a
  splash image when creating an UEFI. Requires bitmap (.bmp) image format.
- **UKI_CMDLINE** -- Variable contains a list of kernel cmdline parameters.
  If the variable is not specified, the parameters from `/etc/cmdline.d/*.conf`
  will be taken. If the directory `/etc/cmdline.d` does not exist, the
  parameters from `/proc/cmdline` will be taken.
- **UKI_VMLINUZ** -- The variable specifies the kernel image filename. If not
  specified, the image will be guessed.
- **UKI_SBAT** -- CSV lines encoded the SBAT metadata for the image.
- **UKI_UEFI_STUB** -- A simple UEFI kernel boot stub. By default,
  systemd-stub(7) is used. But an independent [stubby](https://github.com/puzzleos/stubby)
  can also be used.

## ToDo

- Multi-Profile UKIs
- PE Addons
- UKI TPM PCR Measurements
