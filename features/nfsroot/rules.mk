MODULES_PRELOAD	+= $(NFS_PRELOAD)

$(call require,network)
$(call require,depmod-image)

nfsroot: create network
	@$(MSG) "Adding NFS root support ..."
	@put-tree "$(ROOTDIR)" $(NFS_DATADIR)

depmod-image: nfsroot
