# Feature: locales

This feature adds localization files and directories. By default, the initramfs
locale is `C` and there are no localizations.

All locales must be specified without encoding. UTF-8 is enforced.

## Parameters

- **LOCALES** -- a list of locales that should be copied to the image. The first
  locale in the list will be used as the default locale in the initramfs.
- **LOCALES_TEXTDOMAIN** -- a list of textdomains for initramfs.
