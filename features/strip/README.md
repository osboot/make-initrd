# Feature: strip

This is a system feature. Its purpose is to strip symbol tables, debug
sections, comments, and notes from ELF executables and shared libraries
copied into the initramfs image.

## Parameters

- **STRIP** -- Path to the strip utility (default: `@STRIP@` or `strip`).
- **STRIP_FLAGS** -- Options passed to the strip utility.
