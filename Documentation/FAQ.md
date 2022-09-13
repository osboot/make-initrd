# FAQ

This document provides links to documents that answer some questions about
`make-initrd`.

## How it works ?

Please read the [document about this](HowInitramfsWorks.md).

## How to create a new feature ?

There is a separate [document on this](NewFeature.md).

## The initramfs won't load on my computer. What to do ?

If the resulting initramfs image does not work and you do not know what to do,
then write to me and attach the result of the command:

```bash
$ make-initrd bug-report
```

As a result, an archive will be made with information about the equipment on the
computer. The archive will not include personal data.

## Why isn't systemd being used ?

The systemd is very complex and difficult to debug. The systemd developers also
require many of the latest linux kernel features. For initramfs, these
requirements are superfluous and redundant.
