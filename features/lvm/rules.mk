$(call require,devmapper)

lvm: devmapper
	@echo "Adding LVM support ..."
	@$(PUT_FILE) $(LVM_BIN) $(LVM_CONF)
	@$(PUT_TREE) $(LVM_DATADIR)

pack: lvm
