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
	@if [ -s "$(WORKDIR)/guess/modules" -o -s "$(WORKDIR)/guess/modalias" ]; then \
	   printf 'Guessed modules: '; \
	   tr '\n' ' ' < "$(WORKDIR)/guess/modules"; \
	   tr '\n' ' ' < "$(WORKDIR)/guess/modalias"; \
	   printf '\n'; \
	fi
	@if [ -s "$(WORKDIR)/guess/features" ]; then \
	   printf 'Guessed features: '; \
	   tr '\n' ' ' < "$(WORKDIR)/guess/features"; \
	   printf '\n'; \
	fi

create: depmod-host show-guessed
	@echo "Creating initrd image ..."
	@mkdir -m 755 -p $(verbose) -- $(WORKDIR) $(ROOTDIR)
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
	@echo "Installing image ..."
	@mv -f $(verbose) -- "$(WORKDIR)/initrd.img" "$(IMAGEFILE)"

clean:
	@echo "Removing work directory ..."
	$Qrm -rf -- "$(WORKDIR)"
