ROOTFS_DIRS := $(shell $(FEATURESDIR)/rootfs/bin/create-fstab dirs)
PUT_FEATURE_DIR += $(ROOTFS_DIRS)
