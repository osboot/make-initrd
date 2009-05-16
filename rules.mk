.EXPORT_ALL_VARIABLES:

all: initrd

check-for-root:
	@if [ "$$(id -u)" != 0 ]; then \
	    echo "Only root can do that"; \
	    exit 1; \
	fi

prepare:
	@echo "Preparing work directory ..."
	@mkdir -m 700 -p $(verbose) -- $(WORKDIR) $(ROOTDIR)

depmod: check-for-root prepare
	@echo "Generating module dependencies ..."
	$Qdepmod -a -F "/boot/System.map-$(KERNEL)" "$(KERNEL)"

create: depmod
	@echo "Creating initrd image ..."
	@$(CREATE_INITRD)

run-scripts:
	@echo "Running scripts ..."
	@$(RUN_SCRIPTS)

optional-bootsplash:
	@echo "Building bootsplash ..."
	@$(OPTIONAL_BOOTSPLASH)

pack: prepare
	@echo "Adding more modules and generating module dependencies ..."
	@$(ADD_MODULE) $(MODULES_ADD)
	@$(LOAD_MODULE) $(MODULES_LOAD)
	$Q/sbin/depmod -a -F "/boot/System.map-$(KERNEL)" -b $(ROOTDIR) \
		"$(KERNEL)"

	@echo "Packing image to archive ..."
	@$(PACK_IMAGE)

compress: pack
	@echo "Compressing image ..."
	@$(COMPRESS_IMAGE)

install: pack
	@echo "Installing image ..."
	@mv -f $(verbose) -- "$(WORKDIR)/initrd.img" "$(IMAGEFILE)"

clean:
	@echo "Removing work directory ..."
	$Qrm -rf -- "$(WORKDIR)"

# Load extra rules
-include $(RULESDIR)/*.mk
