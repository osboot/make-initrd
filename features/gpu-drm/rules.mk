# SPDX-License-Identifier: GPL-3.0-or-later

$(call assgin-shell-once,GPUDRM_MODULES,$(FEATURESDIR)/gpu-drm/bin/get-modules)

MODULES_ADD += $(GPUDRM_MODULES)
