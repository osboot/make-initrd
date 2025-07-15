# SPDX-License-Identifier: GPL-3.0-or-later

FAT_MODULES = fs-vfat /fs/nls/
ZFS_MODULES = zfs

MODULES_ADD += $(LUKS_MODULES) drivers/md/raid
MODULES_TRY_ADD += $(FAT_MODULES)
MODULES_TRY_ADD += $(ZFS_MODULES)

PUT_FEATURE_FILES += $(KICKSTART_FILES)
PUT_FEATURE_DIRS += $(KICKSTART_DATADIR)
PUT_FEATURE_PROGS += $(KICKSTART_PROGS)
PUT_FEATURE_PROGS_WILDCARD += $(KICKSTART_PROGS_PATTERNS)

kickstart: create
	@$(VMSG) "Adding kickstart configs..."
	@mkdir -p -- "$(ROOTDIR)/etc/ks.conf.d"
	@[ -z "$(KICKSTART_CONFIGS)" ] || cp -f -- $(KICKSTART_CONFIGS) "$(ROOTDIR)"/etc/ks.conf.d

pack: kickstart
