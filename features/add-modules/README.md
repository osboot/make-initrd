# Feature: add-modules

Feature adds kernel modules to initramfs.

## Parameters

- **MODULES_ADD** -- The parameter contains the names of modules to be added to
  the image. Thay can be loaded by udev if needed when it starts.
- **MODULES_TRY_ADD** -- Like **MODULES_ADD**, but image generation won't fail
  if module is missing.
- **MODULES_PRELOAD** -- The parameter contains the names of modules to be added
  to the image and unconditionally loaded before udev start.
- **MODULES_LOAD** -- The parameter contains the names of modules to be added to
  the image and unconditionally loaded after udev start.
- **RESCUE_MODULES** -- The names of modules to be loaded if emergency shell
  called.
- **BLACKLIST_MODULES** -- The names of modules that should NOT present in the
  image.
- **MODULES_PATTERN_SETS** -- This variable contains names of other variables
  with a list of rules for filtering (see `Pattern sets`).

### Pattern sets

`Pattern set` is a collection of filters that are applied simultaneously to find
kernel modules. The kernel module will be added to the image only when it
satisfies _all_ the rules in the set.

Pattern set format: `field`:`regular-expression`

The `field` can be one of the words: `alias`, `author`, `depends`,
`description`, `filename`, `firmware`, `license`, `name`, `symbol`. The fields
correspond to the metadata in the kernel module. If you want to invert the
expression, then before the `field`, you need to specify the prefix `not-`. For
example `filename` and `not-filename`.

If you specify a regular expression for symbols, keep in mind that it will be
applied to both provided and required symbols.

Example of `rules.mk`:
```make
FILESYSTEM_PATTERN_SET =
FILESYSTEM_PATTERN_SET += alias:^fs-
FILESYSTEM_PATTERN_SET += not-filename:.*/kernel/arch/.*
FILESYSTEM_PATTERN_SET += not-filename:.*/net/.*
MODULES_PATTERN_SETS += FILESYSTEM_PATTERN_SET
```

This set is intended to find all modules whose alias starts with `fs-` that are
not contains `/kernel/arch/` or `/net/` in the path.

The rules can be checked with the `initrd-scanmod` utility. The utility uses a
slightly different rule format.

Example:
```bash
$ cat >/tmp/scanmod.rules <<EOF
alias ^fs-
not-filename .*/kernel/arch/.*
not-filename .*/net/.*
EOF

$ initrd-scanmod --set-version="$(uname -r)" /tmp/scanmod.rules
```
