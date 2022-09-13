# Configuration

Configuration file for make-initrd is `/etc/initrd.mk`. In essence,
the configuration file is a Makefile.  Actually part of it.

You can generate multiple images for one kernel version (for example general,
failsafe and rescue). For that reason `/etc/initrd.mk.d` directory is used.
In this directory each file with the '.mk' extension is regarded as
a configuration file.

## Variables

- **AUTODETECT** - This variable contains the list of subsystems which add
  appropriate kernel modules and features automatically.
- **FEATURES** - The variable lists the features of the image. This list can be
  specified in any order.
- **DISABLE_FEATURES** - The variable lists the features that should NOT present
  in the image.
- **DISABLE_GUESS** - The variable lists guess modules that should NOT run.
- **KERNEL** - The variable specifies kernel version for corresponding modules
  in the image. Default: `$(uname -r)`
- **IMAGE_SUFFIX** - The variable specifies suffix for the image. This parameter
  is useful if multiple initrd images are generated for one kernel version.
- **IMAGEFILE** - The variable specifies output filename for the image.
  Default: `$(BOOTDIR)/initrd-$(KERNEL)$(IMAGE_SUFFIX).img`
- **DEVICES** - The variable lists device files (for example `/dev/sda1`) for
  which you want to add modules.
- **VERBOSE** - Print a message for each action.
- **FIRMWARE_DIRS** - The variable defines the list of directories with firmware
  and microcode.
- **PUT_FILES** - The variable lists the files that need to be put into the initrd
  image. The files are copied with the same path as the original file.
- **PUT_PROGS** - Same as `PUT_FILES` but contains a program names. The full path
  of the executables will be obtained by searching in the directories listed in
  the environment variable `PATH` using the same algorithm as `bash(1)`.
  The `PATH` will be set to `/sbin:/usr/sbin:/usr/local/sbin:/bin:/usr/bin:/usr/local/bin`.
  The reason for this is that such a path is used in the initrd image.
- **PUT_DIRS** - The variable lists directories to be copied into the image.

Additionally, some features also have their own configuration settings.

### Example

The simplest configuration file looks like this:

```
AUTODETECT = all
```

## See also

- The list of [features](Features.md).
- [Guessing the configuration](GuessConfiguration.md).
