# SPDX-License-Identifier: GPL-3.0-or-later
ifeq ($(RDSHELL),)
RDSHELL = shell
endif

ifneq ($(RDSHELL_PASSWORD)$(RDSHELL_COPY_PASSWORD_FROM_USER),)
RDSHELL = login
$(call assign-shell-once,RHSHELL_DIRS,$(FEATURESDIR)/rdshell/bin/add-login dirs)
PUT_FEATURE_DIRS += $(RHSHELL_DIRS)
endif

$(call set-sysconfig,rdshell,RDSHELL_MODE,$(RDSHELL))
