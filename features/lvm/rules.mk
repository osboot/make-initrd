$(call require,devmapper)

lvm: devmapper
	@echo "Adding LVM support ..."
	@$(TOOLSDIR)/put-file $(LVM_BIN) $(LVM_CONF)
	@$(TOOLSDIR)/put-tree $(LVM_DATADIR)

pack: lvm
