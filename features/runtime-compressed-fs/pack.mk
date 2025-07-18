# SPDX-License-Identifier: GPL-3.0-or-later

pre-pack::
	@$(VMSG) "Creating squashfs image ..."
	@$(FEATURESDIR)/runtime-compressed-fs/bin/pack-compressed-fs
