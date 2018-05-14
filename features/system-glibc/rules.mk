system-glibc-libnss: create
	$(Q)[ -n "$(INITRD_NO_LIBNSS)" ] || $(TOOLSDIR)/$@

system-glibc-libgcc_s: create
	$(Q)[ -n "$(INITRD_NO_LIBGCC_S)" ] || $(TOOLSDIR)/$@

system-glibc-pwgr: create
	$(Q)[ -n "$(INITRD_NO_PWGR)" ] || $(TOOLSDIR)/$@

system-glibc-network: create
	$(Q)[ -n "$(INITRD_NO_NETWORK)" ] || $(TOOLSDIR)/$@

pack: system-glibc-libnss
pack: system-glibc-libgcc_s
pack: system-glibc-pwgr
pack: system-glibc-network
