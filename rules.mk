.EXPORT_ALL_VARIABLES:

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

pack: create
	@echo "Packing image to archive ..."
	@$(PACK_IMAGE)

install: pack
	@echo "Installing image ..."
	@mv -f $(verbose) -- "$(WORKDIR)/initrd.img" "$(IMAGEFILE)"

clean:
	@echo "Removing work directory ..."
	$Qrm -rf -- "$(WORKDIR)"

# Load requested features
include $(FEATURES:%=$(RULESDIR)/%.mk)
