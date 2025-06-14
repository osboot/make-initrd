# Feature: Compressed Initramfs

## Overview

This feature introduces support for building a **compressed initramfs** to
significantly reduce memory usage during system boot, especially in
memory-constrained environments. By consolidating most files into a compressed,
read-only squashfs image, the number of files loaded directly into ramfs is
minimized, reducing both memory consumption and page fragmentation.

Two backends are supported:
- squashfs is a compressed read-only filesystem for linux (https://docs.kernel.org/filesystems/squashfs.html);
- erofs - Enhanced Read-Only File System (https://erofs.docs.kernel.org/en/latest/index.html).

## Background

- **Problem:** Traditional initramfs stores all files in ramfs, which is
  uncompressed and allocates at least one page per file, regardless of file
  size. This leads to:
  - High memory usage, especially with many small files.
  - Increased waste on systems with large page sizes due to page fragmentation.

- **Solution:** Move most files into a compressed squashfs/erofs image, keeping
  only a minimal set of executables and libraries outside as a loader. This
  reduces the number of files in ramfs and compresses their contents.

## Parameters

- **RUNTIME_BASE_LAYER** -- If not empty, it determines the type of backend
  image. Maybe `squashfs` or `erofs`. If empty, it is detected by the existence
  of utilities.
- **RUNTIME_LEYER_SQUASHFS_COMPRESS** -- The parameter specifies a compression
  algorithm (default: `xz`).
- **RUNTIME_LAYER_SQUASHFS_ARGS** -- Extra arguments for `mksquashfs`.
- **RUNTIME_LEYER_EFOFS_COMPRESS** -- The parameter specifies a compression
  algorithm.
- **RUNTIME_LEYER_EFOFS_ARGS** -- Extra arguments for `mkfs.erofs`.
