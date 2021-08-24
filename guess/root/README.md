# Guess module: root

This module finds modules needed to access root filesystem device and the
filesystem itself (if they are not added in the kernel).

# Parameters

- **GUESS_MOUNTPOINTS** -- this is a list of mountpoints. The kernel modules will be
  found for devices mounted at these mountpoints.
