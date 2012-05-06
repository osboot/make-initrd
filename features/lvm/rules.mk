$(call require,devmapper)

lvm: devmapper
	@echo "Adding LVM support ..."
	@put-file "$(ROOTDIR)" $(LVM_BIN) $(LVM_CONF)
	@put-tree "$(ROOTDIR)" $(LVM_DATADIR)

pack: lvm
