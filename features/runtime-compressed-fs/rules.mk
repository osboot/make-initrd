# SPDX-License-Identifier: GPL-3.0-or-later

RUNTIME_LAYER_SQUASHFS_MODULES := overlay loop squashfs
RUNTIME_LAYER_EROFS_MODULES    := overlay loop erofs

RUNTIME_COMPRESSED_FS := compressed-fs

GARBAGE += \
	$(ROOTDIR).$(RUNTIME_COMPRESSED_FS) \
	$(WORKDIR)/$(RUNTIME_COMPRESSED_FS)

pre-pack::
	@$(VMSG) "Creating squashfs image ..."
	@$(FEATURESDIR)/runtime-compressed-fs/bin/pack-compressed-fs
