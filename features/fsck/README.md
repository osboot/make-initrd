# Feature: fsck

Feature provides functionality for checking the file system before mounting.
All devices will be checked before mounting. If during verification errors
appear that cannot be fixed, a shell will be opened for repair.

## Boot parameters

- `no-fsck`, `fastboot` disables fsck execution.
