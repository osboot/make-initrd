# make-initrd

<img src="Documentation/logo-big.png" alt="make-initrd logo" width="200" align="right">

`make-initrd` is a uevent-driven initramfs infrastructure based on `udev`.
It is used to build initramfs images and provide the runtime pieces that run
inside those images during early boot.

The project is feature-based: you describe the target image via a set of
features, and the build system resolves ordering and dependencies.

## Quick Start

Build and install:

```bash
./autogen.sh
./configure
make
sudo make install
```

Generate a config from the current system (optional but recommended on first
run):

```bash
sudo make-initrd guess-config | sudo tee /etc/initrd.mk
```

Build initramfs for the running kernel:

```bash
sudo make-initrd -v
```

## Dependencies

### Host-side dependencies (build and image generation)

These are used on the host when building `make-initrd` and when generating
images:

- GNU make
- bash
- coreutils
- pkg-config
- C toolchain (`cc`, headers, standard build tools)
- flex and bison (`lex` and `yacc`)
- `libkmod`/`kmod` (required)
- `udevd` and `udevadm` (required)
- Compression libraries for image tools: `zlib`, `bzip2`, `xz` (`liblzma`), `zstd`
- `libelf` (optional, improves ELF dependency detection)
- `json-c` (optional, supports parsing ELF DLOPEN metadata)
- `scdoc` (optional, for generating man pages)

### Runtime dependencies (inside initramfs image)

These are not generic host build dependencies. They are needed in the generated
initramfs runtime:

- `busybox` for core userspace tools in initramfs
- `libshell` scripts/utilities used by initramfs logic

`make-initrd` can use bundled copies from git submodules or external/system
copies, depending on configure options and host setup.

If you plan to use bundled dependencies, initialize submodules:

```bash
git submodule update --init --recursive
```

## Build Options

Default build:

```bash
./autogen.sh
./configure
make
```

Install to the host system:

```bash
sudo make install
```

Install into a staging directory (for packaging):

```bash
make install DESTDIR=/path/to/stage
```

Developer/in-tree mode:

```bash
./configure --enable-local-build
make
```

## Common Commands

- Build images for configured targets:

```bash
make-initrd
```

- Show guessed config:

```bash
make-initrd guess-config
```

- Show modules needed for a device or mountpoint:

```bash
make-initrd guess-modules /dev/sda1
```

- Collect diagnostic archive:

```bash
make-initrd bug-report
```

## Documentation

- Project overview: [Documentation/README.md](Documentation/README.md)
- Configuration: [Documentation/Configuration.md](Documentation/Configuration.md)
- Guessing logic: [Documentation/GuessConfiguration.md](Documentation/GuessConfiguration.md)
- Boot parameters: [Documentation/BootParameters.md](Documentation/BootParameters.md)
- Initramfs internals: [Documentation/HowInitramfsWorks.md](Documentation/HowInitramfsWorks.md)
- FAQ: [Documentation/FAQ.md](Documentation/FAQ.md)
- Man page source: [Documentation/manpages/make-initrd.1.md](Documentation/manpages/make-initrd.1.md)
- Contributing: [Documentation/Contributing.md](Documentation/Contributing.md)
- Wiki: [https://github.com/osboot/make-initrd/wiki](https://github.com/osboot/make-initrd/wiki)

## Support

`make-initrd` aims to be portable, but is validated on a limited set of systems.

- Operating systems: ALTLinux, Fedora, Gentoo, Ubuntu
- C libraries: GNU libc, musl libc

## License

`make-initrd` is licensed under the GNU General Public License (GPL), version 3.
