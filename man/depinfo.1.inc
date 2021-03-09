.\" Copyright (C) 2021  Alexey Gladkov <gladkov.alexey@gmail.com>
.\"
.\" Additional documentation for the depinfo.
.\"
.\" This file is covered by the GNU General Public License,
.\" which should be included with make-initrd as the file LICENSE.

[NAME]
depinfo \- shows module dependencies

[EXAMPLE]
To see module dependencies and required firmware:
.PP
.in +4n
.EX
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
.EE
.in
.PP

You can also get a simple list of modules:
.PP
.in +4n
.EX
# depnfo -k 5.11.0-rc6 -F b43
module /lib/modules/5.11.0-rc6/kernel/drivers/net/wireless/broadcom/b43/b43.ko.xz
module /lib/modules/5.11.0-rc6/kernel/net/mac80211/mac80211.ko.xz
module /lib/modules/5.11.0-rc6/kernel/net/wireless/cfg80211.ko.xz
module /lib/modules/5.11.0-rc6/kernel/net/rfkill/rfkill.ko.xz
module /lib/modules/5.11.0-rc6/kernel/lib/crypto/libarc4.ko.xz
module /lib/modules/5.11.0-rc6/kernel/drivers/bcma/bcma.ko.xz
module /lib/modules/5.11.0-rc6/kernel/lib/math/cordic.ko.xz
.EE
.in
.PP

Some modules can be compiled into the kernel:
.PP
.in +4n
.EX
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
.EE
.in
.PP

[SEE ALSO]
.BR modinfo (1),
.BR modprobe (1)
