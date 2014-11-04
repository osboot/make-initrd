lxc:
	@echo "Adding LXC support ..."
	@put-file "$(ROOTDIR)" $(LXC_FILES)
	@put-tree "$(ROOTDIR)" $(LXC_DATADIR)

pack: lxc
