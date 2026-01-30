# SPDX-License-Identifier: GPL-3.0-or-later
PHONY += pack-uki install

PUT_FEATURE_PROGS += $(UKI_PROGS)

UKI_EFI_INTERNAL_IMAGE := $(WORKDIR)/linux.efi

pack-uki: pack $(call if-active-feature,ucode compress bootconfig)
	@if [ -f "$(FIRSTCPIO)" ]; then \
	    cat "$(FIRSTCPIO)" "$(OUTFILE)" > "$(OUTFILE)".x; \
	    mv -f -- "$(OUTFILE)".x "$(OUTFILE)"; \
	fi
	@$(VMSG) "Packing UKI image ..."
	@$(FEATURESDIR)/uki/bin/pack-uki

install: pack-uki
	@$(VMSG) "Installing UKI image ..."
	@if [ -f "$(TEMPDIR)/images" ] && grep -Fxqs "$(UKI_IMAGEFILE)" "$(TEMPDIR)/images"; then \
	    echo ""; \
	    echo "An attempt to create two images with the same name. There is possibility"; \
	    echo "that you forgot to define IMAGE_SUFFIX or UKI_IMAGEFILE in one of the config files."; \
	    echo ""; \
	    echo "ERROR: Unable to overwrite the image $(UKI_IMAGEFILE)"; \
	    echo ""; \
	    exit 1; \
	fi >&2
	@$(TOOLSDIR)/show-install-info "$(UKI_EFI_INTERNAL_IMAGE)"
	@mkdir -p -- `dirname "$(UKI_IMAGEFILE)"`
	@chmod 600 -- "$(UKI_EFI_INTERNAL_IMAGE)"
	@mv -f $(verbose3) -- "$(UKI_EFI_INTERNAL_IMAGE)" "$(UKI_IMAGEFILE)"
	@echo "$(UKI_IMAGEFILE)" >> "$(TEMPDIR)/images"

IMAGEFILE = $(UKI_IMAGEFILE)
genimage:
