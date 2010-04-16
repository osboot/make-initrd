# Features include helper: skip a feature which has already loaded
require = $(foreach req,$(1), \
   $(eval include $(filter-out $(MAKEFILE_LIST), \
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
	@echo "Generating module dependencies on host ..."
	$Qdepmod -a -F "/boot/System.map-$(KERNEL)" "$(KERNEL)"

# We should use it here because WORKDIR should be in the same
# context with create target.
show-guessed:
	@if [ -s "$(GUESSDIR)/modules" -o -s "$(GUESSDIR)/modalias" ]; then \
	   printf 'Guessed modules: '; \
	   tr '\n' ' ' < "$(GUESSDIR)/modules"; \
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

rescue-modules: create
	@if [ -n "$(RESCUE_MODULES)" ]; then \
	    echo "Installing resue modules ..."; \
	    $(TOOLSDIR)/add-rescue-modules $(RESCUE_MODULES); \
	fi

pack: create rescue-modules
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
	$Qrm -rf $(verbose) -- "$(ROOTDIR)" "$(GUESSDIR)"
	$Qrmdir -- "$(WORKDIR)"
