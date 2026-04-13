# Feature: overlayfs

Feature adds autodetection support for systems whose root filesystem is mounted
through `overlayfs`.

When the current mount uses filesystem type `overlay`, the guess logic inspects
`lowerdir=` mount options, discovers the underlying block-backed lower
directories, and pulls in the required filesystem and device dependencies for
them.
