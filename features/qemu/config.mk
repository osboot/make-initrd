$(call feature-requires,depmod-image)

GUESSv1_QEMU_SOURCE := $(current-feature-name)
GUESSv1_QEMU_RULE = EXISTS{$(SYSFS_PATH)} ACTION{RUN}=$(FEATURESDIR)/qemu/bin/guess
