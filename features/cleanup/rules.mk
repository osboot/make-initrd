# SPDX-License-Identifier: GPL-3.0-or-later
PHONY += clean

clean: install
	@$(VMSG) "Removing initramfs root directory ..."
	@rm -rf -- "$(ROOTDIR)"
	@$(VMSG) "Removing guess directory ..."
	@rm -rf -- "$(GUESSDIR)"
	@$(VMSG) "Removing data directory ..."
	@rm -rf -- "$(WORKDIR)/root"
	@$(VMSG) "Removing garbage ..."
	@[ -z "$(GARBAGE)" ] || printf '%s\0' $(GARBAGE) | xargs -r0 rm -rf --
	@$(VMSG) "Removing work directory ..."
	@rmdir $(verbose3) -- "$(WORKDIR)"

genimage: clean
