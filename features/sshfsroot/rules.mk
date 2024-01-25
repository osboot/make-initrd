# SPDX-License-Identifier: GPL-3.0-or-later
SSHFS_DIRS := $(shell $(call shell-export-vars) $(FEATURESDIR)/sshfsroot/bin/get-dirs)

MODULES_PRELOAD	+= $(SSHFS_PRELOAD)

PUT_FEATURE_DIRS  += $(SSHFS_DATADIR) $(SSHFS_DIRS)
PUT_FEATURE_PROGS += $(SSHFS_PROGS)
