MODULES_PRELOAD	+= $(NFS_PRELOAD)

nfsroot:
	@echo "Adding NFS root support ..."
	@put-tree "$(ROOTDIR)" $(NFS_DATADIR)

pack: nfsroot
