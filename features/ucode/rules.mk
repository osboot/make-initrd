ucode: $(call if_feature,compress)
	@$(MSG) "Adding CPU microcode ..."
	@$(ADD_UCODE)
	@if [ -s "$(WORKDIR)/ucode.cpio" ]; then \
		cat "$(WORKDIR)/initrd.img" >> "$(WORKDIR)/ucode.cpio"; \
		mv -f $(verbose) -- "$(WORKDIR)/ucode.cpio" "$(WORKDIR)/initrd.img"; \
	fi

install: ucode
