ifeq "$(RDSHELL)" ''
RDSHELL = shell
endif

ifneq "$(RDSHELL_PASSWORD)$(RDSHELL_COPY_PASSWORD_FROM_USER)" ''
RDSHELL = login

RHSHELL_DIRS := $(shell $(shell-export-vars) $(FEATURESDIR)/rdshell/bin/add-login dirs)

PUT_FEATURE_DIRS += $(RHSHELL_DIRS)
endif

$(call set-sysconfig,rdshell,RDSHELL_MODE,$(RDSHELL))
