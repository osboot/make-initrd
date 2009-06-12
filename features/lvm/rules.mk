$(call require,device-mapper)

lvm: device-mapper
	@echo "Adding LVM support ..."
	@$(PUT_FILE) $(LVM_BIN) $(LVM_CONF)
	@$(PUT_TREE) $(LVM_DATADIR)

pack: lvm
