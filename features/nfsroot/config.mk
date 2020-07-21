$(call feature-requires,depmod-image network)

NFS_DATADIR	= $(FEATURESDIR)/nfsroot/data
NFS_PRELOAD	= af_packet nfs
