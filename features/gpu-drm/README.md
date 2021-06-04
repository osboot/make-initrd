# Feature: gpu-drm

The feature adds to the image the modules needed for one or more cards.

## Parameters

- **GPUDRM_CARDS** -- List of patterns cards from `/sys/class/drm/*` for which
  modules need to be added (default value `*`).
