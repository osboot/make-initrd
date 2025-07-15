# SPDX-License-Identifier: GPL-3.0-or-later

MODULES_PRELOAD	+= $(SSHFS_PRELOAD)

PUT_FEATURE_DIRS  += $(SSHFS_DATADIR)
PUT_FEATURE_PROGS += $(SSHFS_PROGS)

sshfs-keys: create
	@$(VMSG) "Adding ssh keys..."
	@$(FEATURESDIR)/sshfsroot/bin/get-dirs

pack: sshfs-keys
