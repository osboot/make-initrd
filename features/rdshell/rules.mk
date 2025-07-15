# SPDX-License-Identifier: GPL-3.0-or-later
ifeq ($(RDSHELL),)
RDSHELL = shell
endif

ifneq ($(RDSHELL_PASSWORD)$(RDSHELL_COPY_PASSWORD_FROM_USER),)
RDSHELL = login
endif

$(call set-sysconfig,rdshell,RDSHELL_MODE,$(RDSHELL))

ifneq ($(RDSHELL_PASSWORD)$(RDSHELL_COPY_PASSWORD_FROM_USER),)
rdshell: create
	@$(VMSG) "Adding root password..."
	@$(FEATURESDIR)/rdshell/bin/add-login

pack: rdshell
endif
