# SPDX-License-Identifier: GPL-3.0-or-later

ifeq ($(INITRD_NO_LIBNSS),)
$(call assign-shell-once,SYSTEM_GLIBC_LIBNSS_FILES,$(FEATURESDIR)/system-glibc/bin/system-glibc-libnss)
PUT_FEATURE_FILES += $(SYSTEM_GLIBC_LIBNSS_FILES)

system-glibc-libnss: create
	@$(VMSG) "Adding libnss data..."
	@mkdir -p -- "$(ROOTDIR)"/etc
	@printf > "$(ROOTDIR)"/etc/nsswitch.conf '%s\n' \
	  "passwd:    files" \
	  "group:     files" \
	  "shadow:    files" \
	  "hosts:     files dns" \
	  "networks:  files" \
	  "rpc:       files" \
	  "services:  files" \
	  "protocols: files"

pack: system-glibc-libnss
endif

ifeq ($(INITRD_NO_LIBGCC_S),)
$(call assign-shell-once,SYSTEM_GLIBC_LIBGCC_S_FILES,$(FEATURESDIR)/system-glibc/bin/system-glibc-libgcc_s)
PUT_FEATURE_FILES += $(SYSTEM_GLIBC_LIBGCC_S_FILES)
endif

ifeq ($(INITRD_NO_PWGR),)
system-glibc-pwgr: create
	@$(VMSG) "Adding users and groups..."
	@$(FEATURESDIR)/system-glibc/bin/system-glibc-pwgr

pack: system-glibc-pwgr
endif

ifeq ($(INITRD_NO_NETWORK),)
system-glibc-network: create
	@$(VMSG) "Adding libc network data..."
	@$(FEATURESDIR)/system-glibc/bin/system-glibc-network

pack: system-glibc-network
endif
