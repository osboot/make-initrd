# How to create a new feature ?

Features serve to extend the functionality of the `make-initrd`.
Features are located in the `features` directory.

## Structure

Let's assume the feature name is `mdadm`. In this case, the directory structure
will be:
```
features/mdadm
 `- README.md
 `- config.mk
 `- rules.mk
 `- data/
```

* README.md - this file contains the documentation of the feature that describes
  the functionality and parameters.
* config.mk - this file is always read. It should only contain the definition of
  feature-specific variables. In this file, the kernel is checked (if
  necessary).
* rules.mk - this file will be used if the `mdadm` feature is used. In this file
  that the real actions should be described.
* data/ - this directory contains a piece of initramfs image that you want to
  put into the image "as is".

### config.mk

Let's say in the image you need to put a mdadm utility, its configuration file
and the data that the utility needs. To do this, we need to create a description
of its resources:
```make
MDAMD_RULES = \
	/lib/udev/rules.d/63-md-raid-arrays.rules \
	/lib/udev/rules.d/64-md-raid-assembly.rules

MDADM_CONF ?= $(SYSCONFDIR)/mdadm.conf

MDADM_PROGS = mdadm
MDADM_FILES = $(wildcard $(MDAMD_RULES) $(MDADM_CONF))
MDADM_DATADIR = $(FEATURESDIR)/mdadm/data
```
Variables do not need to be prefixed like this (`MDADM_`). It must be unique
among other features. Usually a prefix based on the feature name.

The `MDADM_CONF` variable is marked as overridden so that at the time of
initramfs image creation, you can specify a different configuration file.

The `MDADM_DATADIR` contains a path to `data/` directory. In this directory we
will put additional files needed for this feature.

### rules.mk

This file adds files to those required in the initramfs image. There are several
special variables to do this.

* `PUT_FEATURE_DIRS` contains a list of directories whose contents should be
  copied as is.
* `PUT_FEATURE_FILES` contains a list of files that are copied to the image with
  the full path to the file. If a directory is placed in this list, then the
  directory will be copied with all the contents in it.
* `PUT_FEATURE_PROGS` contains a list of program names. These programs will be
  found in the PATH and copied to the image.
* `PUT_FEATURE_PROGS_WILDCARD` contains a list of glob patterns for the full
  path to utilities in the `PATH`.
* `PUT_FEATURE_LIBS` contains a list of libraries that are copied to the image.
  The library will be copied with all linked dependencies.
* `PUT_FEATURE_OPTIONAL_LIBS` this is the same as `PUT_FEATURE_LIBS` but only if
  the library is not found the error does not happen.

```make
PUT_FEATURE_DIRS  += $(MDADM_DATADIR)
PUT_FEATURE_FILES += $(MDADM_FILES)
PUT_FEATURE_PROGS += $(MDADM_PROGS)
```

All binary utilities will be copied along with a libraries with which they are
linked.
