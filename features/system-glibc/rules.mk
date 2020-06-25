system-glibc-libnss: create
	$(Q)[ -n "$(INITRD_NO_LIBNSS)" ] || $(FEATURESDIR)/system-glibc/bin/$@

system-glibc-libgcc_s: create
	$(Q)[ -n "$(INITRD_NO_LIBGCC_S)" ] || $(FEATURESDIR)/system-glibc/bin/$@

system-glibc-pwgr: create
	$(Q)[ -n "$(INITRD_NO_PWGR)" ] || $(FEATURESDIR)/system-glibc/bin/$@

system-glibc-network: create
	$(Q)[ -n "$(INITRD_NO_NETWORK)" ] || $(FEATURESDIR)/system-glibc/bin/$@

pack: system-glibc-libnss
pack: system-glibc-libgcc_s
pack: system-glibc-pwgr
pack: system-glibc-network
