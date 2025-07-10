depinfo(1)

# NAME

depinfo - shows module dependencies

# SYNOPSIS

*depinfo* [options] [module|filename|alias]...

# DESCRIPTION

Displays the full path to module and its dependencies. It also shows the full
path to firmware.

# OPTIONS

*-t, --tree*
	Show dependencies between modules hierarchically.

*-D, --no-deps*
	Don’t show dependence.

*-P, --no-prefix*
	Omit the prefix describing the type of file.

*-M, --no-modules*
	Omit modules from the output.

*-F, --no-firmware*
	Omit firmware from the output.

*-B, --no-builtin*
	Omit builtin modules from the output.

*-k, --set-version=*_VERSION_
	Use _VERSION_ instead of ‘uname -r‘.

*-b, --base-dir=DIR*
	Use _DIR_ as filesystem root for /lib/modules.

*-f, --firmware-dir=*_DIR_
	Use _DIR_ as colon-separated list of firmware directories (default: /lib/firmware/updates:/lib/firmware).

*--use-blacklist*
	Apply blacklist commands in the configuration files.

*--missing-firmware*
Show firmware that could not be found.

*-i, --input=*_FILE_
	Read names from _FILE_.

*-V, --version*
	Show version of program and exit.

*-h, --help*
	Show this text and exit.

# EXAMPLE

To see module dependencies and required firmware:
```
# depinfo -k 5.11.0-rc6 -t b43
module /lib/modules/5.11.0-rc6/kernel/drivers/net/wireless/broadcom/b43/b43.ko.xz
   \\_ firmware /lib/firmware/b43/ucode9.fw
   \\_ firmware /lib/firmware/b43/ucode5.fw
   \\_ firmware /lib/firmware/b43/ucode30_mimo.fw
   \\_ firmware /lib/firmware/b43/ucode33_lcn40.fw
   \\_ firmware /lib/firmware/b43/ucode29_mimo.fw
   \\_ firmware /lib/firmware/b43/ucode26_mimo.fw
   \\_ firmware /lib/firmware/b43/ucode25_mimo.fw
   \\_ firmware /lib/firmware/b43/ucode25_lcn.fw
   \\_ firmware /lib/firmware/b43/ucode24_lcn.fw
   \\_ firmware /lib/firmware/b43/ucode16_mimo.fw
   \\_ firmware /lib/firmware/b43/ucode16_lp.fw
   \\_ firmware /lib/firmware/b43/ucode15.fw
   \\_ firmware /lib/firmware/b43/ucode14.fw
   \\_ firmware /lib/firmware/b43/ucode13.fw
   \\_ firmware /lib/firmware/b43/ucode11.fw
   \\_ module /lib/modules/5.11.0-rc6/kernel/net/mac80211/mac80211.ko.xz
      \\_ module /lib/modules/5.11.0-rc6/kernel/net/wireless/cfg80211.ko.xz
         \\_ module /lib/modules/5.11.0-rc6/kernel/net/rfkill/rfkill.ko.xz
      \\_ module /lib/modules/5.11.0-rc6/kernel/lib/crypto/libarc4.ko.xz
   \\_ module /lib/modules/5.11.0-rc6/kernel/drivers/bcma/bcma.ko.xz
   \\_ module /lib/modules/5.11.0-rc6/kernel/lib/math/cordic.ko.xz
```

You can also get a simple list of modules:
```
# depnfo -k 5.11.0-rc6 -F b43
module /lib/modules/5.11.0-rc6/kernel/drivers/net/wireless/broadcom/b43/b43.ko.xz
module /lib/modules/5.11.0-rc6/kernel/net/mac80211/mac80211.ko.xz
module /lib/modules/5.11.0-rc6/kernel/net/wireless/cfg80211.ko.xz
module /lib/modules/5.11.0-rc6/kernel/net/rfkill/rfkill.ko.xz
module /lib/modules/5.11.0-rc6/kernel/lib/crypto/libarc4.ko.xz
module /lib/modules/5.11.0-rc6/kernel/drivers/bcma/bcma.ko.xz
module /lib/modules/5.11.0-rc6/kernel/lib/math/cordic.ko.xz
```

Some modules can be compiled into the kernel:
```
# depinfo -k 5.11.0-rc6 -F btrfs
module /lib/modules/5.11.0-rc6/kernel/fs/btrfs/btrfs.ko.xz
module /lib/modules/5.11.0-rc6/kernel/crypto/blake2b_generic.ko.xz
builtin sha256_generic
module /lib/modules/5.11.0-rc6/kernel/crypto/xxhash_generic.ko.xz
builtin crc32c_generic
module /lib/modules/5.11.0-rc6/kernel/lib/libcrc32c.ko.xz
builtin crc32c_generic
module /lib/modules/5.11.0-rc6/kernel/lib/raid6/raid6_pq.ko.xz
module /lib/modules/5.11.0-rc6/kernel/crypto/xor.ko.xz
```

To find out what firmware could not be found for the kernel modules:
```
# depinfo --missing-firmware rtlwifi
module /lib/modules/6.15.5-gentoo-dist/kernel/drivers/net/wireless/realtek/rtlwifi/rtlwifi.ko
   \\_ module /lib/modules/6.15.5-gentoo-dist/kernel/net/wireless/cfg80211.ko
      \\_ missing-firmware regulatory.db
      \\_ missing-firmware regulatory.db
      \\_ missing-firmware regulatory.db.p7s
      \\_ missing-firmware regulatory.db.p7s
      \\_ module /lib/modules/6.15.5-gentoo-dist/kernel/net/rfkill/rfkill.ko
   \\_ module /lib/modules/6.15.5-gentoo-dist/kernel/net/mac80211/mac80211.ko
      \\_ module /lib/modules/6.15.5-gentoo-dist/kernel/lib/crypto/libarc4.ko
```

# AUTHOR

Written by Alexey Gladkov.

# BUGS

Report bugs to the authors.

# SEE ALSO

_modinfo_(1) _modprobe_(1)
