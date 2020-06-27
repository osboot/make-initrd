MODULES_PRELOAD	+= $(NFS_PRELOAD)

PUT_FEATURE_DIRS  += $(NFS_DATADIR)

$(call require,network)
$(call require,depmod-image)
