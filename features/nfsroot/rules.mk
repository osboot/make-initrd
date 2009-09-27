nfsroot:
	@echo "Adding NFS root support ..."
	@$(PUT_TREE) $(NFS_DATADIR)

pack: nfsroot
