# FAQ

This document provides links to documents that answer some questions about
`make-initrd`.

## How it works ?

Please read the [document about this](HowItWorks.md).

## How to create a new feature ?

There is a separate [document on this](NewFeature.md).

## Why isn't systemd being used?

The systemd is very complex and difficult to debug. The systemd developers also
require many of the latest linux kernel features. For initramfs, these
requirements are superfluous and redundant.
