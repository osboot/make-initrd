# SPDX-License-Identifier: GPL-3.0-or-later

$(call assign-shell-once,FSCK_RUNTIME_FILES,$(FEATURESDIR)/fsck/bin/find-files)

PUT_FEATURE_DIRS  += $(FSCK_DATADIR)
PUT_FEATURE_FILES += $(FSCK_FILES) $(FSCK_RUNTIME_FILES)

ifeq ($(FSCK_ARGS),)
FSCK_ARGS = -p

VENDOR =
ifneq ($(wildcard /etc/os-release),)
VENDOR := $(shell sed -n -e 's/^ID=//p' /etc/os-release)
endif

ifeq ($(VENDOR),altlinux)
FSCK_ARGS += -y
endif
endif

$(call set-sysconfig,fsck,FSCK_ARGS,$(FSCK_ARGS))
