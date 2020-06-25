ifeq "$(RDSHELL)" ''
RDSHELL = shell
endif
ifneq "$(RDSHELL_PASSWORD)$(RDSHELL_COPY_PASSWORD_FROM_USER)" ''
RDSHELL = login
endif

rdshell: create $(call if_feature,system-glibc-pwgr)
	@$(MSG) "Adding initrd shell support ($(RDSHELL) mode) ..."
	@mkdir -p -- "$(ROOTDIR)/etc/sysconfig"
	@echo "RDSHELL_MODE=$(RDSHELL)" >"$(ROOTDIR)/etc/sysconfig/rdshell"
	@$(FEATURESDIR)/rdshell/bin/add-login

pack: rdshell
