# SPDX-License-Identifier: GPL-3.0-or-later

$(call assgin-shell-once,SSHFS_DIRS,$(FEATURESDIR)/sshfsroot/bin/get-dirs)

MODULES_PRELOAD	+= $(SSHFS_PRELOAD)

PUT_FEATURE_DIRS  += $(SSHFS_DATADIR) $(SSHFS_DIRS)
PUT_FEATURE_PROGS += $(SSHFS_PROGS)
