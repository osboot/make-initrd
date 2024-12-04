# SPDX-License-Identifier: GPL-3.0-or-later

PUT_FEATURE_PROGS += $(UKI_PROGS)

UKI_EFI_INTERNAL_IMAGE := $(WORKDIR)/linux.efi

pack-uefi: pack $(call if-active-feature,ucode bootconfig)
	@$(VMSG) "Packing UEFI image ..."
	@$(FEATURESDIR)/uki/bin/pack-uefi

install: pack-uefi
	@$(MSG) 'Used features: $(USED_FEATURES)'
	@$(MSG_N) 'Packed modules: '
	@find $(ROOTDIR)/lib/modules/$(KERNEL) -type f \( -name '*.ko'  -o -name '*.ko.*' \) -printf '%f\n' 2>/dev/null | \
	    sed -e 's/\.ko\(\.[^\.]\+\)\?$$//' | sort -u | tr '\n' ' '
	@printf '\n'
	@if [ -f "$(TEMPDIR)/images" ] && grep -Fxqs "$(UKI_IMAGEFILE)" "$(TEMPDIR)/images"; then \
	    echo ""; \
	    echo "An attempt to create two images with the same name. There is possibility" >&2; \
	    echo "that you forgot to define IMAGE_SUFFIX or IMAGEFILE in one of the config files." >&2; \
	    echo "" >&2; \
	    echo "ERROR: Unable to overwrite the image $(UKI_IMAGEFILE)" >&2; \
	    echo "" >&2; \
	    exit 1; \
	else \
	    $(VMSG) "Installing UKI image ..."; \
	    $(MSG) "Unpacked size: `du -sh "$(WORKDIR)/img" |cut -f1 ||:`"; \
	    $(MSG) "Image size: `du -sh "$(UKI_EFI_INTERNAL_IMAGE)" |cut -f1 ||:`"; \
	    chmod 600 -- "$(UKI_EFI_INTERNAL_IMAGE)"; \
	    mv -f $(verbose) -- "$(UKI_EFI_INTERNAL_IMAGE)" "$(UKI_IMAGEFILE)"; \
	    echo "$(UKI_IMAGEFILE)" >> "$(TEMPDIR)/images"; \
	fi

genimage: install
	@$(MSG) "Image is saved as $(UKI_IMAGEFILE)"
	@echo
