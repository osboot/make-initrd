GPUDRM_MODULES := $(shell $(shell-export-vars) $(FEATURESDIR)/gpu-drm/bin/get-modules)

MODULES_ADD += $(GPUDRM_MODULES)
