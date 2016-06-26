# Features include helper: skip a feature which has already loaded
# and skip globally disabled features
require = \
	$(foreach req,$(1), \
		$(eval include $(filter-out \
			$(MAKEFILE_LIST) $(foreach exl,$(DISABLE_FEATURES),$(realpath $(exl:%=$(FEATURESDIR)/%/rules.mk))), \
			$(realpath $(req:%=$(FEATURESDIR)/%/rules.mk)))))

genimage: install
	@echo
	@echo "Image is saved as $(IMAGEFILE)"
	@echo

check-for-root:
	@if [ "$$(id -u)" != 0 ]; then \
	    echo "Only root can do that"; \
	    exit 1; \
	fi

depmod-host: check-for-root
	@if [ -z "$$IGNORE_DEPMOD" ]; then \
	   echo "Generating module dependencies on host ..."; \
	   depmod -a -F "/boot/System.map-$(KERNEL)" "$(KERNEL)"; \
	fi

# We should use it here because WORKDIR should be in the same
# context with create target.
show-guessed:
	@if [ -s "$(GUESSDIR)/modules" -o -s "$(GUESSDIR)/modalias" ]; then \
	   printf 'Guessed modules: '; \
	   sed -r 's,^.*/([^/]+)\.ko.*$$,\1,' < "$(GUESSDIR)/modules" | tr '\n' ' '; \
	   tr '\n' ' ' < "$(GUESSDIR)/modalias"; \
	   printf '\n'; \
	fi
	@if [ -s "$(GUESSDIR)/features" ]; then \
	   printf 'Guessed features: '; \
	   tr '\n' ' ' < "$(GUESSDIR)/features"; \
	   printf '\n'; \
	fi

create: depmod-host show-guessed
	@echo "Creating initrd image ..."
	@mkdir -m 755 -p $(verbose) -- $(ROOTDIR)
	@$(TOOLSDIR)/create-initrd

show-modules:
	@printf 'Packed modules: '
	@find $(ROOTDIR)/lib/modules/$(KERNEL) -type f \( -name '*.ko'  -o -name '*.ko.*' \) -printf '%f\n' 2>/dev/null | \
	    sed -e 's/\.ko\(\.[^\.]\+\)\?$$//' | sort | tr '\n' ' '
	@printf '\n'

pre-pack: show-modules

pack: create
	@echo "Packing image to archive ..."
	@$(TOOLSDIR)/pack-image

install: pack
	@if [ -f "$(TEMPDIR)/images" ] && fgrep -xqs "$(IMAGEFILE)" "$(TEMPDIR)/images"; then \
	    echo ""; \
	    echo "An attempt to create two images with the same name. There is possibility" >&2; \
	    echo "that you forgot to define IMAGE_SUFFIX or IMAGEFILE in one of the config files." >&2; \
	    echo "" >&2; \
	    echo "ERROR: Unable to overwrite the image $(IMAGEFILE)" >&2; \
	    echo "" >&2; \
	    exit 1; \
	else \
	    echo "Installing image ..."; \
	    chmod 600 -- "$(WORKDIR)/initrd.img"; \
	    mv -f $(verbose) -- "$(WORKDIR)/initrd.img" "$(IMAGEFILE)"; \
	    echo "$(IMAGEFILE)" >> "$(TEMPDIR)/images"; \
	fi

clean:
	@echo "Removing work directory ..."
	$Qrm -rf -- "$(ROOTDIR)" "$(GUESSDIR)" "$(WORKDIR)/initcpio"
	$Qrmdir -- "$(WORKDIR)"
