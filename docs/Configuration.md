# Configuration

Configuration file for make-initrd is `/etc/initrd.mk`. In essence, the configuration file is a Makefile.  Actually part of it.

You can generate multiple images for one kernel version (for example general, failsafe and rescue). For that reason `/etc/initrd.mk.d` directory is used.  In this directory each file with the '.mk' extension is regarded as a configuration file.

## Variables

- **AUTODETECT** - This variable contains the list of subsystems which add appropriate kernel modules and features automatically.
- **FEATURES** - This parameter lists the features of the image. This list can be specified in any order.
- **DISABLE_FEATURES** - This parameter lists the features that should NOT present in the image.
- **DISABLE_GUESS** - lists guess modules that should NOT run.
- **KERNEL** - Specifies kernel version for corresponding modules in the image.
Default: `$(uname -r)`
- **IMAGE_SUFFIX** - Specifies suffix for the image. This parameter is useful if multiple initrd images are generated for one kernel version.
- **IMAGEFILE** - Specifies output filename for the image.
Default: `$(BOOTDIR)/initrd-$(KERNEL)$(IMAGE_SUFFIX).img`
- **DEVICES** - List of device files (for example `/dev/sda1`) for which you want to add modules.
- **VERBOSE** - Print a message for each action.
- **FIRMWARE_DIRS** - This parameter defines the list of directories with firmware and microcode.
- **PUT_FILES** - This parameter describes the list of files that need to be put in the image initrd.  The files are copied with the same path as the original file.
- **PUT_DIRS** - This parameter describes the list of directories to be copied into the image.

Additionally, some features also have their own configuration settings.
