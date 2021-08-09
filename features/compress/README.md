# Feature: compress

This is a system feature. Its purpose is to compress initramfs image.
If the previous image has been compressed, it will use the same compression algorithm.

## Parameters

- **COMPRESS** -- Determines compress method for the image. Valid values are: `gzip`, `bzip2`, 'lz4', `lzma`, `lzo`, 'xz', and 'zstd'.
