lvm: device-mapper $(ROOTDIR)/etc/udev/rules.d/99-lvm-initrd.rules
	@$(PUT_FILE) $(LVM_BIN) $(LVM_CONF)

$(ROOTDIR)/etc/udev/rules.d/99-lvm-initrd.rules: create
	@echo 'SUBSYSTEM=="block",ACTION=="add|change"	RUN+="/sbin/lvm vgscan"' > $@
	@echo 'SUBSYSTEM=="block",ACTION=="add|change"	RUN+="/sbin/lvm vgchange -ay"' >> $@
