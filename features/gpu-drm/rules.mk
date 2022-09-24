GPUDRM_MODULES := $(shell $(shell_export_vars) $(FEATURESDIR)/gpu-drm/bin/get-modules)

MODULES_ADD += $(GPUDRM_MODULES)
