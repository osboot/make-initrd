nfsroot:
	@echo "Adding NFS root support ..."
	@$(TOOLSDIR)/put-tree $(NFS_DATADIR)

pack: nfsroot
