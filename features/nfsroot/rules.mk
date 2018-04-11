MODULES_PRELOAD	+= $(NFS_PRELOAD)

nfsroot:
	@$(MSG) "Adding NFS root support ..."
	@put-tree "$(ROOTDIR)" $(NFS_DATADIR)

pack: nfsroot
