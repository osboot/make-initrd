#### Guess modules

- **common** - adds features for general use.  Don't disable this feature unless you know what you are doing.
- **device** - finds modules needed to access to `DEVICES`.
- **root** - finds modules needed to access root filesystem device and the filesystem itself (if they are not added in the kernel);
- **resume** - finds modules needed to access hibernation image device. This is useful when the device is encrypted or in an unexpected location.
- **net** - adds kernel modules for network devices (netboot).
- **keyboard** - adds modules for the keyboard.
- **rdshell** - gives the ability to get shell in initrd.
