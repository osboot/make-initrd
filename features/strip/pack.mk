# SPDX-License-Identifier: GPL-3.0-or-later

pre-pack::
	@$(VMSG) "Stripping binaries ..."
	@$(STRIP_IMAGE)
