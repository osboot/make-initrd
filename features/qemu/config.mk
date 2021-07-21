$(call feature-requires,depmod-image)

GUESSv1_QEMU_RULE = EXISTS{/sys} ACTION{RUN}=$(FEATURESDIR)/qemu/bin/guess
