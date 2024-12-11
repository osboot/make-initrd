# SPDX-License-Identifier: GPL-3.0-or-later
PHONY += compress

compress: pack
	@$(VMSG) "Compressing image ..."
	@$(COMPRESS_IMAGE)

install: compress
