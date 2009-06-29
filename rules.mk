.EXPORT_ALL_VARIABLES:

# Features include helper: skip a feature which has already loaded
require = $(foreach req,$(1),$(eval include $(filter-out $(MAKEFILE_LIST),$(req:%=$(FEATURESDIR)/%/rules.mk))))

all: install

check-for-root:
	@if [ "$$(id -u)" != 0 ]; then \
	    echo "Only root can do that"; \
	    exit 1; \
	fi

depmod-host: check-for-root
	@echo "Generating module dependencies on host ..."
	$Qdepmod -a -F "/boot/System.map-$(KERNEL)" "$(KERNEL)"

create: depmod-host
	@echo "Creating initrd image ..."
	@mkdir -m 700 -p $(verbose) -- $(WORKDIR) $(ROOTDIR)
	@$(CREATE_INITRD)

rescue-modules: create
	@if [ -n "$(RESCUE_MODULES)" ]; then \
	    echo "Installing resue modules ..."; \
	    $(ADD_RESCUE_MODULES) $(RESCUE_MODULES); \
	fi

pack: create rescue-modules
	@echo "Packing image to archive ..."
	@$(PACK_IMAGE)

install: pack
	@echo "Installing image ..."
	@mv -f $(verbose) -- "$(WORKDIR)/initrd.img" "$(IMAGEFILE)"

clean:
	@echo "Removing work directory ..."
	$Qrm -rf -- "$(WORKDIR)"

# Load requested features
include $(call require,$(FEATURES))
